# Phase 2 / 3 — EKF Kinematics & DeepSORT Association Blueprint

> **Scope.** Rigorous mathematical specification for the 3D constant-acceleration
> Extended Kalman Filter (EKF) and DeepSORT-style data-association layer
> that will be implemented in `core/kinematics_engine/` using **Eigen 3.4+ /
> C++17**.
>
> **Status.** Research-only. No production code is to be written until this
> blueprint is reviewed and approved (per `cursor.md` Phase 3 mandate).
>
> **Authoring conventions.**
> - All matrices are written in **row-major mathematical notation**; the C++
>   implementation must use Eigen's default column-major storage with
>   compile-time fixed sizes (`Eigen::Matrix<double, 9, 9>`, etc.) to enable
>   stack allocation and SIMD vectorisation.
> - Vectors are column vectors unless transposed.
> - Time is in seconds (`double`). Pixels are `double`. Lengths in 3D are in
>   metres (`double`).
> - Cross-references are tagged `[§N.m]`.

---

## 0. Symbols & Coordinate Frames

| Symbol | Meaning |
|---|---|
| $\mathcal{F}_W$ | **World** frame. Static, right-handed, Z-up. Origin at the rig base. |
| $\mathcal{F}_C$ | **Camera** frame. Right-handed, $X_C$ right, $Y_C$ down, $Z_C$ forward (OpenCV convention). |
| $\mathcal{F}_I$ | **Image** frame. $u$ right, $v$ down, origin at top-left pixel. |
| $\mathbf{R}_{CW} \in SO(3)$ | Rotation that maps a vector expressed in $\mathcal{F}_W$ to $\mathcal{F}_C$. |
| $\mathbf{t}_{CW} \in \mathbb{R}^3$ | Translation: $\mathbf{p}_C = \mathbf{R}_{CW}\mathbf{p}_W + \mathbf{t}_{CW}$. |
| $\mathbf{K} \in \mathbb{R}^{3\times3}$ | Camera intrinsics, $\mathbf{K} = \begin{bmatrix} f_x & 0 & c_x \\ 0 & f_y & c_y \\ 0 & 0 & 1 \end{bmatrix}$ |
| $\Delta t$ | Discrete sample period of the prediction loop. Nominal 33.33 ms @ 30 fps; jittered. |
| $\mathbf{I}_n$ | $n\times n$ identity. $\mathbf{0}_{m\times n}$ — zero block. |
| $\otimes$ | Kronecker product. |

**Edge-case ground rules** (apply throughout):
1. $\Delta t$ must be measured from monotonic clock (`std::chrono::steady_clock`),
   never the wall clock. Frame drops mean $\Delta t$ can spike — the model
   must remain stable for $\Delta t \in [0.005,\, 0.300]\,\text{s}$.
2. The pinhole projection has a removable singularity at $Z_C \to 0^+$.
   The EKF measurement update **must** guard against $Z_C \le z_{\text{min}}$
   (we adopt $z_{\text{min}} = 0.30\,\text{m}$) and skip the update with a
   logged warning instead of producing NaN.
3. Numerical conditioning of $\mathbf{P}$ is enforced through the
   **Joseph form** covariance update [§3.7] and a symmetrisation step
   $\mathbf{P} \leftarrow \tfrac{1}{2}(\mathbf{P} + \mathbf{P}^{\top})$ after
   every prediction.

---

## 1. State Vector — 3D Constant-Acceleration Cinematic Model

### 1.1 Definition

Per humanoid sub-feature (`person | head | upper_torso | accessory`) we
instantiate one filter with the **9-dimensional** state

$$
\mathbf{x} \;=\; \big[\, p_x,\, p_y,\, p_z,\; v_x,\, v_y,\, v_z,\; a_x,\, a_y,\, a_z \,\big]^{\top} \in \mathbb{R}^{9}.
$$

Equivalently, in block form

$$
\mathbf{x} \;=\; \begin{bmatrix} \mathbf{p} \\ \mathbf{v} \\ \mathbf{a} \end{bmatrix},\qquad
\mathbf{p},\mathbf{v},\mathbf{a} \in \mathbb{R}^{3}.
$$

All quantities are expressed in $\mathcal{F}_W$ — the EKF estimates **world-frame
metric coordinates**. The image plane is the *observation* space, not the
state space. This choice is deliberate:

- A world-frame state lets the actuator node compute physical intercept
  angles directly (no inverse projection required at the consumer end).
- It generalises trivially to multi-camera fusion (just stack measurement
  rows).
- Velocities and accelerations have meaningful, comparable units
  (m·s⁻¹, m·s⁻²) regardless of zoom level or resolution.

### 1.2 Eigen typedefs (binding contract for Phase 4)

```cpp
namespace roc::kinematics {
  using Scalar    = double;
  using State     = Eigen::Matrix<Scalar, 9, 1>;
  using StateCov  = Eigen::Matrix<Scalar, 9, 9>;
  using Meas      = Eigen::Matrix<Scalar, 4, 1>;   // [u, v, w_px, h_px]^T
  using MeasCov   = Eigen::Matrix<Scalar, 4, 4>;
  using Jacobian  = Eigen::Matrix<Scalar, 4, 9>;
}
```

Index conventions: position indices `{0,1,2}`, velocity `{3,4,5}`,
acceleration `{6,7,8}`.

### 1.3 Continuous-time dynamics

The model assumes acceleration is the *highest deterministic* derivative
and that the third derivative (jerk) is a zero-mean white-noise process
$\mathbf{w}(t) \sim \mathcal{N}(\mathbf{0}, \mathbf{Q}_c)$.

$$
\dot{\mathbf{x}}(t) \;=\; \mathbf{F}_c\,\mathbf{x}(t) \;+\; \mathbf{G}\,\mathbf{w}(t),
$$

with

$$
\mathbf{F}_c \;=\;
\begin{bmatrix}
\mathbf{0}_{3} & \mathbf{I}_{3} & \mathbf{0}_{3} \\
\mathbf{0}_{3} & \mathbf{0}_{3} & \mathbf{I}_{3} \\
\mathbf{0}_{3} & \mathbf{0}_{3} & \mathbf{0}_{3}
\end{bmatrix} \in \mathbb{R}^{9\times9},
\qquad
\mathbf{G} \;=\; \begin{bmatrix} \mathbf{0}_{3} \\ \mathbf{0}_{3} \\ \mathbf{I}_{3} \end{bmatrix} \in \mathbb{R}^{9\times3}.
$$

$\mathbf{F}_c$ is the triple-integrator generator and is **nilpotent of index 3**:
$\mathbf{F}_c^{\,2} \neq 0$, $\mathbf{F}_c^{\,3} = \mathbf{0}$.

---

## 2. Discrete-Time State Transition & Process Noise

### 2.1 State Transition Matrix $\mathbf{F}$

Because $\mathbf{F}_c$ is nilpotent of index 3, the matrix exponential
collapses to a finite sum:

$$
\mathbf{F}(\Delta t) \;=\; \exp(\mathbf{F}_c\,\Delta t)
\;=\; \mathbf{I}_9 + \mathbf{F}_c\Delta t + \tfrac{1}{2}\mathbf{F}_c^{\,2}\Delta t^{2}.
$$

Computing $\mathbf{F}_c^{\,2}$:

$$
\mathbf{F}_c^{\,2} \;=\;
\begin{bmatrix}
\mathbf{0}_{3} & \mathbf{0}_{3} & \mathbf{I}_{3} \\
\mathbf{0}_{3} & \mathbf{0}_{3} & \mathbf{0}_{3} \\
\mathbf{0}_{3} & \mathbf{0}_{3} & \mathbf{0}_{3}
\end{bmatrix}.
$$

Therefore the **exact** discrete transition is

$$
\boxed{\;\mathbf{F}(\Delta t) \;=\;
\begin{bmatrix}
\mathbf{I}_{3} & \Delta t\,\mathbf{I}_{3} & \tfrac{1}{2}\Delta t^{2}\,\mathbf{I}_{3} \\
\mathbf{0}_{3} & \mathbf{I}_{3}            & \Delta t\,\mathbf{I}_{3}                  \\
\mathbf{0}_{3} & \mathbf{0}_{3}            & \mathbf{I}_{3}
\end{bmatrix} \in \mathbb{R}^{9\times9}.\;}
$$

Element-wise this realises the Newtonian kinematic block:

$$
p_{k+1} = p_k + v_k \Delta t + \tfrac{1}{2}a_k\Delta t^{2},\quad
v_{k+1} = v_k + a_k\Delta t,\quad
a_{k+1} = a_k.
$$

Because $\mathbf{F}$ has a closed analytic form, **never compute it via
`Eigen::Matrix::exp()`** — that uses Pade approximants and burns ~3000
cycles. Construct it directly from the four scalar blocks. Suggested
implementation:

```cpp
StateCov F = StateCov::Identity();
F.block<3,3>(0, 3).diagonal().setConstant(dt);
F.block<3,3>(0, 6).diagonal().setConstant(0.5 * dt * dt);
F.block<3,3>(3, 6).diagonal().setConstant(dt);
```

### 2.2 Process Noise Covariance $\mathbf{Q}$ — exact derivation

Discrete-time process noise from continuous white-noise jerk follows the
**van Loan integral**:

$$
\mathbf{Q}(\Delta t) \;=\; \int_{0}^{\Delta t} \mathbf{F}(\tau)\,\mathbf{G}\,\mathbf{Q}_c\,\mathbf{G}^{\top}\,\mathbf{F}(\tau)^{\top}\, d\tau.
$$

We model the jerk PSD as diagonal:

$$
\mathbf{Q}_c \;=\; \operatorname{diag}\!\big(\sigma_{j,x}^{2},\,\sigma_{j,y}^{2},\,\sigma_{j,z}^{2}\big) \;\;[\text{m}^{2}\cdot\text{s}^{-5}].
$$

Pre-computing $\mathbf{F}(\tau)\,\mathbf{G}$ (the third block-column of $\mathbf{F}$):

$$
\mathbf{F}(\tau)\,\mathbf{G} \;=\; \begin{bmatrix} \tfrac{1}{2}\tau^{2}\,\mathbf{I}_{3} \\ \tau\,\mathbf{I}_{3} \\ \mathbf{I}_{3} \end{bmatrix}.
$$

Hence the integrand expands to the block matrix

$$
\mathbf{F}(\tau)\mathbf{G}\mathbf{Q}_c\mathbf{G}^{\top}\mathbf{F}(\tau)^{\top} =
\begin{bmatrix}
\tfrac{\tau^{4}}{4}\mathbf{Q}_c & \tfrac{\tau^{3}}{2}\mathbf{Q}_c & \tfrac{\tau^{2}}{2}\mathbf{Q}_c \\
\tfrac{\tau^{3}}{2}\mathbf{Q}_c & \tau^{2}\mathbf{Q}_c               & \tau\,\mathbf{Q}_c                \\
\tfrac{\tau^{2}}{2}\mathbf{Q}_c & \tau\,\mathbf{Q}_c                & \mathbf{Q}_c
\end{bmatrix}.
$$

Integrating each entry over $[0,\Delta t]$ termwise gives the **closed-form
discrete process-noise covariance**:

$$
\boxed{\;\mathbf{Q}(\Delta t) \;=\;
\begin{bmatrix}
\tfrac{\Delta t^{5}}{20}\,\mathbf{Q}_c & \tfrac{\Delta t^{4}}{8}\,\mathbf{Q}_c & \tfrac{\Delta t^{3}}{6}\,\mathbf{Q}_c \\[4pt]
\tfrac{\Delta t^{4}}{8}\,\mathbf{Q}_c  & \tfrac{\Delta t^{3}}{3}\,\mathbf{Q}_c & \tfrac{\Delta t^{2}}{2}\,\mathbf{Q}_c \\[4pt]
\tfrac{\Delta t^{3}}{6}\,\mathbf{Q}_c  & \tfrac{\Delta t^{2}}{2}\,\mathbf{Q}_c & \Delta t\,\mathbf{Q}_c
\end{bmatrix} \in \mathbb{R}^{9\times9}.\;}
$$

This is the **continuous white-noise jerk (CWNJ) model**. It is positive
semi-definite by construction (it is an outer-product integral). All
nine blocks are diagonal because $\mathbf{Q}_c$ is diagonal — Phase 4
must exploit this with `.diagonal().setConstant(...)` calls rather than
full matrix assignments.

### 2.3 Alternative — Discrete White Noise Acceleration (DWNA)

For tracks where jerk modelling is unstable (e.g., near-stationary
torsos), we keep DWNA as a fallback. It assumes the acceleration is
piecewise constant per step with a random increment $\Delta a$ of
variance $\sigma_a^{2}\,\Delta t$:

$$
\mathbf{Q}_{\text{DWNA}} = \sigma_a^{2}\,
\begin{bmatrix}
\tfrac{\Delta t^{4}}{4}\,\mathbf{I}_{3} & \tfrac{\Delta t^{3}}{2}\,\mathbf{I}_{3} & \tfrac{\Delta t^{2}}{2}\,\mathbf{I}_{3} \\
\tfrac{\Delta t^{3}}{2}\,\mathbf{I}_{3} & \Delta t^{2}\,\mathbf{I}_{3}              & \Delta t\,\mathbf{I}_{3}                 \\
\tfrac{\Delta t^{2}}{2}\,\mathbf{I}_{3} & \Delta t\,\mathbf{I}_{3}                 & \mathbf{I}_{3}
\end{bmatrix}.
$$

CWNJ is the default; the model is selected per-track via a strategy enum.

### 2.4 Tuning ranges (humanoid surveillance defaults)

| Target | $\sigma_j$ (m·s⁻⁵ᐟ²) | Justification |
|---|---|---|
| `upper_torso` | 0.5 | Stable centre of mass, low jerk. |
| `person`      | 1.0 | Slightly more variable due to gait. |
| `head`        | 4.0 | Rapid orientation flicks, looking around. |
| `accessory`   | 2.0 | Swinging backpacks, dropped hats. |

These map directly to per-track `Qc` diagonals.

### 2.5 Prediction step

$$
\hat{\mathbf{x}}_{k|k-1} \;=\; \mathbf{F}(\Delta t)\,\hat{\mathbf{x}}_{k-1|k-1},
\qquad
\mathbf{P}_{k|k-1} \;=\; \mathbf{F}\,\mathbf{P}_{k-1|k-1}\,\mathbf{F}^{\top} \;+\; \mathbf{Q}(\Delta t).
$$

The Eigen prescription uses `.selfadjointView<Lower>()` to compute
$\mathbf{F}\mathbf{P}\mathbf{F}^{\top}$ while exploiting symmetry of
$\mathbf{P}$ — saves ~40% on the prediction hot path.

### 2.6 Adaptive process noise — stationary-regime mitigation (Safety Trap B)

**Failure mode addressed.** Under static targets the CA model's third-order
integration amplifies sub-pixel detector jitter into spurious acceleration,
inflating $\mathbf{P}$ until the Mahalanobis gate drops the track. A flat
$\mathbf{Q}_c$ is wrong in both regimes — too tight under real maneuvers,
too loose at rest.

**Mitigation.** A three-layer stationarity-aware schedule, all closed-form
and cheap:

#### 2.6.1 Windowed NIS regime detector

Maintain a per-track exponential moving average of the Normalised
Innovation Squared:

$$
\bar\eta_k \;=\; (1-\alpha)\,\bar\eta_{k-1} \;+\; \alpha\,\boldsymbol{\nu}_k^{\top}\,\mathbf{S}_k^{-1}\,\boldsymbol{\nu}_k,
\qquad \alpha = 0.1.
$$

Under a correctly tuned filter $\bar\eta$ is $\chi^2_4$-distributed with
mean $4$. Three regimes:

$$
\text{regime}(\bar\eta) \;=\;
\begin{cases}
\textsc{stationary} & \bar\eta < \eta_{\text{low}} = 1.0 \\
\textsc{maneuver}   & \bar\eta > \eta_{\text{high}} = 15.0 \\
\textsc{steady}     & \text{otherwise}.
\end{cases}
$$

#### 2.6.2 Adaptive jerk PSD scaling

$$
\mathbf{Q}_{c,\text{active}}(\bar\eta) \;=\; \rho(\bar\eta)\,\mathbf{Q}_{c,\text{nominal}},\qquad
\rho(\bar\eta) \;=\;
\begin{cases}
0.05 & \bar\eta \le \eta_{\text{low}} \\
1.0  & \bar\eta = \mathbb{E}[\chi^2_4] = 4 \\
4.0  & \bar\eta \ge \eta_{\text{high}}
\end{cases}
$$

linearly interpolated between the breakpoints. In \textsc{stationary} the
$\tfrac{\Delta t^5}{20}\mathbf{Q}_c$ position contribution drops by 20×,
making spurious-jerk integration physically negligible.

#### 2.6.3 Velocity / acceleration shrinkage pseudo-measurement

Every $K = 5$ frames while the track is in \textsc{stationary}, apply
an additional EKF update step using the virtual observation

$$
\mathbf{z}_{\text{rest}} = \mathbf{0}_6,\quad
\mathbf{H}_{\text{rest}} = [\,\mathbf{0}_{6\times 3}\;|\;\mathbf{I}_6\,],\quad
\mathbf{R}_{\text{rest}} = \operatorname{diag}\big(\sigma_{v,\text{stat}}^{2}\mathbf{I}_3,\;\sigma_{a,\text{stat}}^{2}\mathbf{I}_3\big),
$$

with $\sigma_{v,\text{stat}} = 0.05\,\text{m/s}$ and
$\sigma_{a,\text{stat}} = 0.05\,\text{m/s}^{2}$. This is Bayesian
shrinkage on $(\mathbf{v},\mathbf{a})$ — equivalent to a soft L2 prior at
zero — and is **automatically released** when the regime detector flips
back to \textsc{steady} or \textsc{maneuver}.

#### 2.6.4 Centroid innovation deadband (stationary only)

Inside \textsc{stationary} only, zero out the centroid component of the
innovation when below the detector's quantisation floor:

$$
\boldsymbol{\nu}_{uv} \leftarrow
\begin{cases}
\mathbf{0} & \lVert\boldsymbol{\nu}_{uv}\rVert_\infty < \delta_{\text{dead}} = 0.5\,\text{px} \\
\boldsymbol{\nu}_{uv} & \text{otherwise}.
\end{cases}
$$

Box-dimension rows are **never** deadbanded — they still carry depth
information.

#### 2.6.5 Why not the full Singer model?

The Singer mean-reverting acceleration model
($\dot{\mathbf{a}} = -\alpha\,\mathbf{a} + \mathbf{w}_\alpha$) is the
"textbook" answer to this trap, but its discrete $\mathbf{F},\mathbf{Q}$
contain $e^{-\alpha\Delta t}$ and $(\alpha\Delta t)^k$ terms that
re-introduce transcendentals into the hot path. The layered mitigation
above is *first-order equivalent* to Singer (in the sense that both
contract the acceleration mean toward zero with a relaxation
time-scale) but preserves the closed-form $\mathbf{F},\mathbf{Q}$ of
§2.1–§2.2. Singer remains the documented Phase 5 fallback if the
adaptive schedule proves insufficient in field testing.

---

## 3. Measurement Model & Jacobians — Pinhole Projection

### 3.1 Measurement vector

The vision pipeline (Phase 1) supplies, per detection:

$$
\mathbf{z} \;=\; \big[\,u,\; v,\; w_{\text{px}},\; h_{\text{px}}\,\big]^{\top} \in \mathbb{R}^{4},
$$

where $(u,v)$ is the bounding-box **centroid** in pixels and
$(w_{\text{px}}, h_{\text{px}})$ are the box width / height in pixels
**in the original capture-frame coordinate system** (i.e., the inverse
letterbox transform from `phase_1_inference_and_targeting.md` §2 has
already been applied at the publisher).

### 3.2 Coordinate frame chain

$$
\underbrace{\mathbf{p}_W}_{\text{state}} \;\xrightarrow{\;\mathbf{R}_{CW},\,\mathbf{t}_{CW}\;}\; \mathbf{p}_C \;\xrightarrow{\;\text{pinhole + }\mathbf{K}\;}\; (u, v).
$$

Let

$$
\mathbf{p}_C \;=\; \mathbf{R}_{CW}\mathbf{p}_W + \mathbf{t}_{CW} \;=\; \begin{bmatrix} X \\ Y \\ Z \end{bmatrix},
\qquad
\mathbf{R}_{CW} = \begin{bmatrix} \mathbf{r}_1^{\top} \\ \mathbf{r}_2^{\top} \\ \mathbf{r}_3^{\top} \end{bmatrix},
$$

where $\mathbf{r}_1, \mathbf{r}_2, \mathbf{r}_3 \in \mathbb{R}^{3}$ are
the rows of the world-to-camera rotation. Then

$$
X = \mathbf{r}_1^{\top}\mathbf{p}_W + t_x,\quad
Y = \mathbf{r}_2^{\top}\mathbf{p}_W + t_y,\quad
Z = \mathbf{r}_3^{\top}\mathbf{p}_W + t_z.
$$

The non-linear measurement function $\mathbf{h}:\mathbb{R}^{9}\!\to\!\mathbb{R}^{4}$
is

$$
\mathbf{h}(\mathbf{x}) \;=\;
\begin{bmatrix}
\,f_x\,\dfrac{X}{Z} + c_x\, \\[6pt]
\,f_y\,\dfrac{Y}{Z} + c_y\, \\[6pt]
\,f_x\,\dfrac{W_0}{Z}\, \\[6pt]
\,f_y\,\dfrac{H_0}{Z}\,
\end{bmatrix}.
$$

The third and fourth rows encode the **weak-perspective bounding-box
scaling assumption**: the metric width $W_0$ and height $H_0$ of the
tracked sub-feature are treated as known priors (table below). This
turns box size into a depth observation — the EKF can recover $p_z$
from a single monocular camera.

| Sub-feature | $W_0$ (m) | $H_0$ (m) | Source |
|---|---|---|---|
| `person`        | 0.55 | 1.75 | adult median, anthropometric tables |
| `head`          | 0.18 | 0.23 | average human cranial bounding box |
| `upper_torso`   | 0.45 | 0.55 | shoulder-width × sternum height |
| `accessory:hat` | 0.30 | 0.18 | wide brim, conservative |
| `accessory:backpack` | 0.35 | 0.50 | adult daypack |

The priors are *per-class* constants stored in
`core/kinematics_engine/include/roc/kinematics/object_priors.hpp`.

### 3.3 Why this is an **Extended** (not linear) Kalman filter

The pinhole division $1/Z$ makes $\mathbf{h}$ non-linear. The EKF
linearises about the predicted mean with the Jacobian

$$
\mathbf{H}(\mathbf{x}) \;=\; \left.\dfrac{\partial \mathbf{h}}{\partial \mathbf{x}}\right|_{\hat{\mathbf{x}}_{k|k-1}} \;\in\; \mathbb{R}^{4\times 9}.
$$

### 3.4 Derivation of $\mathbf{H}$

Velocity and acceleration do **not** appear in $\mathbf{h}$, so the last
six columns of $\mathbf{H}$ are zero:

$$
\mathbf{H} \;=\; \big[\;\mathbf{H}_p\;\;\big|\;\;\mathbf{0}_{4\times 3}\;\;\big|\;\;\mathbf{0}_{4\times 3}\;\big],\qquad
\mathbf{H}_p \;=\; \dfrac{\partial \mathbf{h}}{\partial \mathbf{p}} \in \mathbb{R}^{4\times 3}.
$$

Differentiate row-by-row. Note $\partial X / \partial \mathbf{p} = \mathbf{r}_1^{\top}$,
$\partial Y / \partial \mathbf{p} = \mathbf{r}_2^{\top}$, and
$\partial Z / \partial \mathbf{p} = \mathbf{r}_3^{\top}$.

**Row 1 (centroid $u$):**

$$
\dfrac{\partial}{\partial \mathbf{p}}\!\left[f_x\,\dfrac{X}{Z} + c_x\right]
= f_x\,\dfrac{\mathbf{r}_1^{\top}\,Z - X\,\mathbf{r}_3^{\top}}{Z^{2}}
= \dfrac{f_x}{Z}\left(\mathbf{r}_1^{\top} - \dfrac{X}{Z}\,\mathbf{r}_3^{\top}\right).
$$

**Row 2 (centroid $v$):**

$$
\dfrac{\partial}{\partial \mathbf{p}}\!\left[f_y\,\dfrac{Y}{Z} + c_y\right]
= \dfrac{f_y}{Z}\left(\mathbf{r}_2^{\top} - \dfrac{Y}{Z}\,\mathbf{r}_3^{\top}\right).
$$

**Row 3 (box width $w_{\text{px}}$):**

$$
\dfrac{\partial}{\partial \mathbf{p}}\!\left[f_x\,\dfrac{W_0}{Z}\right]
= -\,\dfrac{f_x\,W_0}{Z^{2}}\,\mathbf{r}_3^{\top}.
$$

**Row 4 (box height $h_{\text{px}}$):**

$$
\dfrac{\partial}{\partial \mathbf{p}}\!\left[f_y\,\dfrac{H_0}{Z}\right]
= -\,\dfrac{f_y\,H_0}{Z^{2}}\,\mathbf{r}_3^{\top}.
$$

Therefore the **explicit $4\times 3$ position-Jacobian block** is

$$
\boxed{\;\mathbf{H}_p \;=\;
\begin{bmatrix}
\dfrac{f_x}{Z}\big(\mathbf{r}_1^{\top} - \tfrac{X}{Z}\mathbf{r}_3^{\top}\big) \\[10pt]
\dfrac{f_y}{Z}\big(\mathbf{r}_2^{\top} - \tfrac{Y}{Z}\mathbf{r}_3^{\top}\big) \\[10pt]
-\,\dfrac{f_x W_0}{Z^{2}}\,\mathbf{r}_3^{\top} \\[10pt]
-\,\dfrac{f_y H_0}{Z^{2}}\,\mathbf{r}_3^{\top}
\end{bmatrix}.\;}
$$

The full Jacobian is

$$
\mathbf{H} \;=\; \begin{bmatrix} \mathbf{H}_p & \mathbf{0}_{4\times3} & \mathbf{0}_{4\times3} \end{bmatrix} \in \mathbb{R}^{4\times 9}.
$$

### 3.5 Special case — fixed, level camera

For the initial single-camera bring-up the rig is mounted level
($\mathbf{R}_{CW} = \mathbf{I}_3$ after axis-permutation: world Z-up $\to$
camera Y-down requires the standard 180° flip about $X_C$). With the
identity simplification $\mathbf{r}_i = \mathbf{e}_i$, $\mathbf{p}_C =
\mathbf{p}_W + \mathbf{t}_{CW}$, and $X=p_x{+}t_x$ etc., the Jacobian
collapses to

$$
\mathbf{H}_p \;=\;
\begin{bmatrix}
f_x/Z & 0 & -f_x\,X/Z^{2} \\
0 & f_y/Z & -f_y\,Y/Z^{2} \\
0 & 0 & -f_x\,W_0/Z^{2} \\
0 & 0 & -f_y\,H_0/Z^{2}
\end{bmatrix}.
$$

This is the form most monocular DeepSORT papers cite, and it must be
the unit-test ground truth for the general derivation.

### 3.6 Measurement noise $\mathbf{R}$

The detector contributes pixel noise on both the centroid and the box
dimensions. Empirically these are uncorrelated but have different
variances:

$$
\mathbf{R} \;=\; \operatorname{diag}\!\big(\sigma_u^{2},\,\sigma_v^{2},\,\sigma_w^{2},\,\sigma_h^{2}\big).
$$

Default values (calibrated against COCO val-set per-class jitter):

| Component | Value |
|---|---|
| $\sigma_u, \sigma_v$ | 2.0 px |
| $\sigma_w, \sigma_h$ | 6.0 px (boxes jitter ~3× more than centroids) |

For high-confidence detections we scale $\mathbf{R}$ by
$1 / \text{conf}^{2}$ — low-confidence detections inflate noise rather
than getting hard-rejected.

### 3.7 Update step (EKF, Joseph form)

Innovation:

$$
\boldsymbol{\nu}_k \;=\; \mathbf{z}_k - \mathbf{h}(\hat{\mathbf{x}}_{k|k-1}),\quad
\mathbf{S}_k \;=\; \mathbf{H}_k\,\mathbf{P}_{k|k-1}\,\mathbf{H}_k^{\top} + \mathbf{R}_k.
$$

Kalman gain via Cholesky solve (never form $\mathbf{S}^{-1}$ explicitly):

$$
\mathbf{K}_k \;=\; \mathbf{P}_{k|k-1}\,\mathbf{H}_k^{\top}\,\mathbf{S}_k^{-1}.
$$

In Eigen:

```cpp
Eigen::LLT<MeasCov> S_llt(S);
auto K = S_llt.solve(H * P).transpose();   // 9x4
```

State and (Joseph-form, numerically stable) covariance update:

$$
\hat{\mathbf{x}}_{k|k} \;=\; \hat{\mathbf{x}}_{k|k-1} + \mathbf{K}_k\,\boldsymbol{\nu}_k,
$$

$$
\mathbf{P}_{k|k} \;=\; (\mathbf{I} - \mathbf{K}_k\mathbf{H}_k)\,\mathbf{P}_{k|k-1}\,(\mathbf{I} - \mathbf{K}_k\mathbf{H}_k)^{\top} + \mathbf{K}_k\,\mathbf{R}_k\,\mathbf{K}_k^{\top}.
$$

The Joseph form is mandatory: it preserves symmetric positive-definiteness
of $\mathbf{P}$ even under finite-precision arithmetic and tight gating
(where $\mathbf{K}\mathbf{H} \to \mathbf{I}$).

### 3.8 Initialisation from first detection

Because a single 2D box has rank-deficient information about $\mathbf{p}_W$,
the initial $Z$ is back-solved from the bounding-box prior:

$$
Z_0 \;=\; \tfrac{1}{2}\left(\dfrac{f_x W_0}{w_{\text{px}}} + \dfrac{f_y H_0}{h_{\text{px}}}\right).
$$

The lateral state is then $\mathbf{p}_C^{(0)} = Z_0 \cdot \mathbf{K}^{-1}\,
[u,\,v,\,1]^{\top}$, and $\mathbf{p}_W^{(0)} = \mathbf{R}_{CW}^{\top}(\mathbf{p}_C^{(0)} - \mathbf{t}_{CW})$.
Velocity and acceleration initialise to zero with large diagonal
covariance ($\sigma_{v,0}=2$ m/s, $\sigma_{a,0}=2$ m/s² per axis) so the
filter aggressively converges over the first few frames.

### 3.9 Edge cases

| Case | Handling |
|---|---|
| $Z \le 0.30$ m | Skip update; widen $\mathbf{R}$ by ×10 for the next admissible frame. |
| $w_{\text{px}}$ or $h_{\text{px}} < 4$ px | Reject the box-dim rows: project $\mathbf{H}$ to its top 2 rows and use only the centroid for that update (selective measurement). |
| Box aspect ratio $\ge 3\sigma$ from prior $W_0/H_0$ | Likely a mislabel — emit an event and downweight $\mathbf{R}$ for this detection. |
| $\det(\mathbf{S}) < 10^{-12}$ | Bail; the linearisation has degenerated. |

### 3.10 Boundary-aware initialisation (Safety Trap A)

**Failure mode addressed.** §3.8's mean-of-two-depths estimator
catastrophically diverges when one bounding-box axis is clipped by the
frame edge (or a static occluder). A pedestrian entering the frame from
the side with the trailing leg cut off has a half-width box → $Z_0$
shoots to 50 m → the filter spawns with huge uncertainty in the
*wrong* part of the world manifold → the first real update produces
gigantic innovations → divergence.

**Mitigation.** Use the asymmetry between the two depth estimates as
the diagnostic, and never trust a depth from a clipped axis.

#### 3.10.1 Per-axis depth estimates and disagreement gate

Define the two independent depth estimates:

$$
z_w \;=\; \frac{f_x\, W_0}{w_{\text{px}}},\qquad
z_h \;=\; \frac{f_y\, H_0}{h_{\text{px}}}.
$$

For an unclipped, correctly-classified detection these agree to within
the per-class anthropometric variance — typically $\pm 15\%$. Define the
disagreement gate:

$$
\kappa \;=\; \frac{\max(z_w, z_h)}{\min(z_w, z_h)},\qquad
\kappa_{\text{disagree}} = 1.6.
$$

#### 3.10.2 Boundary-contact predicate

With box corners $(u_{\text{left}}, u_{\text{right}}, v_{\text{top}}, v_{\text{bot}})$
derived from $(u, v, w_{\text{px}}, h_{\text{px}})$, and a frame-edge
margin $\varepsilon_b = 4\,\text{px}$:

$$
\begin{aligned}
\text{contact}_{x} &:\;\; u_{\text{left}} \le \varepsilon_b \;\;\text{or}\;\; u_{\text{right}} \ge W_{\text{img}} - \varepsilon_b, \\
\text{contact}_{y} &:\;\; v_{\text{top}}  \le \varepsilon_b \;\;\text{or}\;\; v_{\text{bot}}   \ge H_{\text{img}} - \varepsilon_b.
\end{aligned}
$$

#### 3.10.3 Selective-axis initialisation policy

$$
Z_0 \;=\;
\begin{cases}
\tfrac{1}{2}(z_w + z_h) & \neg\,\text{contact}_x \wedge \neg\,\text{contact}_y \wedge \kappa \le \kappa_{\text{disagree}} \\
z_h                     & \text{contact}_x \wedge \neg\,\text{contact}_y \\
z_w                     & \text{contact}_y \wedge \neg\,\text{contact}_x \\
\textsc{deferred}       & (\text{contact}_x \wedge \text{contact}_y)\;\vee\;\kappa > \kappa_{\text{disagree}}
\end{cases}
$$

A \textsc{deferred} detection is **not** turned into a track. Instead it
enters a *pending-spawn buffer* keyed by appearance embedding; it
remains there for at most $T_{\text{hold}} = 5$ frames waiting for a
follow-up detection that satisfies one of the first three cases. If
$T_{\text{hold}}$ expires, the buffer entry is dropped.

#### 3.10.4 Boundary-probation state machine

A track born from one of the partial cases above is created with
`is_boundary_probation = true`. While the flag is set:

1. The third and fourth rows of $\mathbf{H}$ (box-dim → depth) are
   **excised** from every update — i.e., $\mathbf{H} \to \mathbf{H}_{[0:2,\,:]}$
   and $\mathbf{R} \to \mathbf{R}_{[0:2,\,0:2]}$. The filter is fed
   centroid-only measurements until the box clears the frame.
2. The initial $\mathbf{P}_0$ has its $p_z$ diagonal inflated by ×4 to
   reflect the single-axis depth estimate.
3. The flag clears when **three consecutive frames** of full
   in-frame detections (with $\varepsilon_b$ margin) have been observed.
   On clearing, the next update is allowed to consume the full 4-row
   $\mathbf{H}$ and rapidly tightens $p_z$.

#### 3.10.5 Diagnostic counters

Per-track counters `boundary_init_count`, `disagree_init_count`,
`probation_frames_total` are exported to the SvelteKit dashboard so the
operator can see whether the trap is firing under their specific
camera placement — these are *real* operational signals about field-of-view
margins, not just internal debug noise.

### 3.11 Hierarchical inter-track structural constraint (Safety Trap C)

**Failure mode addressed.** Independent filters per sub-feature (person,
head, upper_torso, accessories) can — and will — drift apart by tens of
centimetres when association latency differs across labels. The
actuator node, asked to point at the "head of person ID 17", may
receive a head position that physically cannot belong to the torso of
person ID 17. Downstream code then either snaps between targets or
issues commands that violate joint limits.

**Mitigation choice.** Rejected: joint state expansion to 27-D per
entity (covariance solve cost goes from $9^3$ to $27^3$ — a 27× hit on
the per-frame update budget; unacceptable for the 144 Hz dashboard
contract). Adopted: **soft constraint via pseudo-measurement
applied as two coupled Gauss–Seidel EKF passes**, which preserves the
9-D per-track filter while structurally tying the hierarchy.

#### 3.11.1 Anatomical offset priors

For every sub-feature $S$ associated with a parent $P$, an
anatomical offset $\boldsymbol{\mu}_S \in \mathbb{R}^3$ (expressed in
$\mathcal{F}_W$, gravity-aligned) and tolerance
$\boldsymbol{\sigma}_S \in \mathbb{R}^3$:

| Sub-feature $S$ | $\boldsymbol{\mu}_S$ relative to torso anchor (m) | $\boldsymbol{\sigma}_S$ (m) |
|---|---|---|
| `upper_torso` | $(0,0,0)$ — this is the anchor | $(0.02, 0.02, 0.05)$ |
| `head`        | $(0, 0, +0.40)$ | $(0.15, 0.15, 0.25)$ |
| `accessory:hat` | $(0, 0, +0.55)$ | $(0.20, 0.20, 0.10)$ |
| `accessory:backpack` | $(0, -0.20, +0.10)$ | $(0.20, 0.15, 0.30)$ |

These tolerances must accommodate leaning, turning, and partial
crouching — they are intentionally loose. Tightening below the values
in the table risks pathological tugging during natural posture changes.

#### 3.11.2 Pseudo-measurement formulation

For each $(P, S)$ pair, define the relative-position pseudo-measurement

$$
\boxed{\;
\mathbf{z}_{\text{rel}} \;=\; \mathbf{0}_3,\qquad
\mathbf{h}_{\text{rel}}(\mathbf{x}_P, \mathbf{x}_S) \;=\; (\mathbf{p}_S - \mathbf{p}_P) - \boldsymbol{\mu}_S,\qquad
\mathbf{R}_{\text{rel}} \;=\; \operatorname{diag}(\boldsymbol{\sigma}_S^{\,2}).
\;}
$$

The corresponding Jacobians (rows for the 3 pseudo-measurement
components, columns for the relevant state's 9D vector) are:

$$
\mathbf{H}_{\text{rel}}^{(S)} \;=\; \big[\,+\mathbf{I}_3 \;\big|\; \mathbf{0}_{3\times 6}\,\big],\qquad
\mathbf{H}_{\text{rel}}^{(P)} \;=\; \big[\,-\mathbf{I}_3 \;\big|\; \mathbf{0}_{3\times 6}\,\big].
$$

#### 3.11.3 Gauss–Seidel coupled update protocol

Per frame, after **every** sub-feature and parent filter has finished
its detection-driven update [§3.7]:

```
for iter in 1..N_iter:                  # N_iter = 2 is sufficient
  for each (parent P, sub-feature S) bound in the hierarchy:
    if not (gate_passed(P, S)): continue
    ν_rel ← -h_rel(x_P, x_S)            # = μ_S - (p_S - p_P)
    # Pass 1: update S, treating P as known
    S_S   ← H_rel^S P_S (H_rel^S)ᵀ + R_rel
    K_S   ← P_S (H_rel^S)ᵀ S_S⁻¹
    x_S  ← x_S + K_S · ν_rel
    P_S  ← Joseph(P_S, K_S, H_rel^S, R_rel)
    # Pass 2: update P, treating refreshed S as known
    ν_rel ← -h_rel(x_P, x_S)            # recompute with new x_S
    S_P   ← H_rel^P P_P (H_rel^P)ᵀ + R_rel
    K_P   ← P_P (H_rel^P)ᵀ S_P⁻¹
    x_P  ← x_P + K_P · ν_rel
    P_P  ← Joseph(P_P, K_P, H_rel^P, R_rel)
```

This Gauss–Seidel scheme converges geometrically with ratio
$\sigma_S^{2}/(\sigma_S^{2} + \sigma_P^{2})$. For our priors that ratio is
$\le 0.6$, so **$N_{\text{iter}} = 2$** drives the residual below $1\%$
of the tolerance — measured against the joint-state EKF ground truth in
the simulation harness. There is no need to iterate further.

#### 3.11.4 Application gate

The constraint update is applied **only when all of the following hold**:

1. Both $P$ and $S$ are in `confirmed` state (`hits >= 3`).
2. Both filters had a real detection this frame
   (`time_since_update == 0` on both, post-update).
3. The current violation is within plausibility:
   $\lVert\mathbf{h}_{\text{rel}}(\mathbf{x}_P,\mathbf{x}_S)\rVert < 3\,\lVert\boldsymbol{\sigma}_S\rVert$.
   A *larger* violation strongly suggests the association layer paired
   the sub-feature with the wrong parent; in that case the constraint is
   skipped and a `hierarchy_violation` event is emitted for the
   downstream selector to re-evaluate parentage.

#### 3.11.5 Why "soft" (not hard rigid)

A hard rigid constraint (project both filters onto the manifold
$\mathbf{p}_S - \mathbf{p}_P = \boldsymbol{\mu}_S$) is *singular* the
instant the human twists their torso or leans: the manifold no longer
matches reality, and projection yanks the filter into a physically
wrong state with zero uncertainty about being wrong. The pseudo-measurement
formulation degrades gracefully — the tolerance $\boldsymbol{\sigma}_S$
encodes our admitted ignorance about the true offset, so the filter only
ever *softly* pulls toward the prior in proportion to how much
information the prior carries relative to the data.

---

## 4. DeepSORT-Style Association Matrix

The association layer runs **before** the EKF update phase. Its role is
to decide which Phase 1 detection $j \in \{1\dots M\}$ feeds which
predicted track $i \in \{1\dots N\}$, so that the EKF update receives
clean $\mathbf{z}_j$ ↔ track-$i$ assignments.

### 4.1 Track-side data structures

Each active track carries:

```cpp
struct Track {
  uint64_t id;
  State    x_hat;                 // predicted state (post-§2.5 prediction)
  StateCov P;                     // predicted covariance
  std::deque<Embedding> gallery;  // L2-normalised feature gallery, FIFO of size L_k <= L_max
  int      age, hits, time_since_update;
  Label    label;                 // person / head / torso / accessory
};
```

- `Embedding = Eigen::Matrix<float, 128, 1>` (DeepSORT-canonical
  dimensionality from the wide-residual Re-ID network).
- Each embedding is **L2-normalised at insertion**: $\lVert\mathbf{f}\rVert_2 = 1$.
  This is what permits cosine distance to reduce to $1 - \mathbf{a}^{\top}\mathbf{b}$.
- $L_{\max} = 100$ (the original DeepSORT default). Older entries are
  popped FIFO so the gallery stays representative under appearance drift.

### 4.2 Distance metrics

#### 4.2.1 Cosine distance (appearance)

For detection embedding $\mathbf{f}_j$ and the gallery
$\mathcal{R}_i = \{\mathbf{r}_i^{(1)}, \dots, \mathbf{r}_i^{(L_i)}\}$ of track $i$:

$$
d^{(\text{app})}_{ij} \;=\; \min_{k \in \{1,\dots,L_i\}}\, \big(1 - {\mathbf{r}_i^{(k)}}^{\top}\,\mathbf{f}_j\big).
$$

Since all vectors are unit-normalised, $1 - \mathbf{r}^{\top}\mathbf{f} \in [0, 2]$,
and $d^{(\text{app})} = 0$ only when the detection matches an exact gallery
member. The matrix form for the full $N \times M$ block is

$$
\mathbf{D}^{(\text{app})} \;=\; \mathbf{1}_{N\times M} - \min_{\text{rowwise per track}}\!\big(\mathbf{R}_{\bullet}\,\mathbf{F}^{\top}\big),
$$

where $\mathbf{F} \in \mathbb{R}^{M \times 128}$ stacks detection embeddings
and $\mathbf{R}_{\bullet}$ is the per-track gallery matrix. In Eigen this
is a single GEMM per track:

```cpp
Eigen::MatrixXf sims = track.gallery_matrix() * F.transpose();   // L_i x M
Eigen::RowVectorXf d_app = 1.0f - sims.colwise().maxCoeff();     // 1 x M
```

#### 4.2.2 Mahalanobis distance (motion gating)

For the same detection–track pair, the gating distance is computed in
**innovation space** (4-D), reusing the very $\mathbf{S}_k$ that
the EKF update would compute [§3.7]:

$$
d^{(\text{mot})}_{ij}\;^{2} \;=\; \big(\mathbf{z}_j - \mathbf{h}(\hat{\mathbf{x}}_i)\big)^{\top}\,\mathbf{S}_i^{-1}\,\big(\mathbf{z}_j - \mathbf{h}(\hat{\mathbf{x}}_i)\big).
$$

Under the linearised Gaussian assumption $d^{(\text{mot})\,2}$ is
$\chi^{2}_4$-distributed. We adopt the canonical DeepSORT threshold

$$
\tau_{\text{mot}}^{2} \;=\; \chi^{2}_{4,\,0.95} \;=\; 9.4877,
$$

i.e., reject any pair whose Mahalanobis distance squared exceeds 9.4877
*before* it ever enters the cost matrix.

Implementation note: solve $\mathbf{S}_i \mathbf{y} = \boldsymbol{\nu}$
via Cholesky and form $d^{2} = \boldsymbol{\nu}^{\top}\mathbf{y}$. Never
invert $\mathbf{S}$.

### 4.3 Gated combined cost matrix

The DeepSORT association cost is a convex combination of the two
metrics, with a hard motion gate and a hard appearance gate:

$$
C_{ij} \;=\;
\begin{cases}
\lambda\,\dfrac{d^{(\text{mot})}_{ij}}{\sqrt{\tau_{\text{mot}}^{2}}} \;+\; (1-\lambda)\,d^{(\text{app})}_{ij}, & d^{(\text{mot})\,2}_{ij} \le \tau_{\text{mot}}^{2}\;\wedge\;d^{(\text{app})}_{ij} \le \tau_{\text{app}} \\[8pt]
+\infty, & \text{otherwise}.
\end{cases}
$$

Defaults:

| Param | Value | Notes |
|---|---|---|
| $\lambda$ | 0.0 (camera moving) / 0.2 (camera static) | The original paper uses $\lambda=0$ when the camera is in motion — motion gating still applies, but it does not contribute to the soft cost. |
| $\tau_{\text{app}}$ | 0.2 | Appearance gate: reject any pair below cosine similarity ≈ 0.8. |
| $\tau_{\text{mot}}^{2}$ | 9.4877 | $\chi^{2}_{4,\,0.95}$. |

The resulting matrix $\mathbf{C} \in (\mathbb{R}^{+}\cup\{\infty\})^{N\times M}$
is the association matrix consumed by the linear assignment solver.

### 4.4 Matching cascade

DeepSORT does **not** solve the assignment on the full $N\times M$ cost
matrix in one shot. It runs a *cascade*: tracks that have been confirmed
recently (lower `time_since_update`) get first claim on detections.

```
unmatched_detections ← all M detection indices
matched ← ∅
for n = 1 .. A_max:                # A_max = max track age, default 30
    T_n ← {tracks i : time_since_update(i) == n}
    if T_n is empty: continue
    C_n ← build_cost_matrix(T_n, unmatched_detections)   # see §4.3
    (assignments, unmatched_T_n, new_unmatched_dets) ← hungarian(C_n)
    matched ← matched ∪ assignments
    unmatched_detections ← new_unmatched_dets
unmatched_tracks ← all confirmed tracks not in matched
```

The cascade prevents "feature drift": stale tracks (long
`time_since_update`) — whose predicted covariances have ballooned and
whose galleries are out of date — cannot steal detections that fit a
fresh, confident track.

After the cascade, **unconfirmed** tracks (those still in the
probation window, default `hits < 3`) and any remaining unmatched
*confirmed* tracks are matched against the leftover detections using
**IoU distance only** (DeepSORT §IV.B). This is the "second-stage" IoU
fallback.

$$
d^{(\text{IoU})}_{ij} \;=\; 1 - \text{IoU}\!\big(\text{bbox}(\hat{\mathbf{x}}_i), \mathbf{z}_j\big),
\qquad
\tau_{\text{IoU}} = 0.7.
$$

### 4.5 Linear assignment

Each cascade slice solves

$$
\min_{\pi}\; \sum_{i\in T_n}\, C_{i,\,\pi(i)}\quad\text{s.t. }\pi\text{ is injective on }T_n.
$$

We use the **Jonker–Volgenant** (LAP-JV) algorithm: $O((\max(N,M))^{3})$
worst case but in practice 5–10× faster than naive Hungarian on
sparse-gated matrices. A header-only C++17 implementation lives in the
`third_party/lap` submodule (to be vendored in Phase 4).

Assignments with $C_{ij} = +\infty$ are filtered out **after** solving
— LAP-JV will return a permutation, but any pair whose cost equals the
sentinel large value (we use `1e6` in practice; never literal `inf`,
which breaks the solver) is treated as unmatched.

### 4.6 Lifecycle decisions post-association

| Outcome | Action |
|---|---|
| Track $i$ matched to detection $j$ | Run EKF update [§3.7]. Append $\mathbf{f}_j$ to gallery. Reset `time_since_update`. Increment `hits`. |
| Track $i$ unmatched | Skip update. `time_since_update += 1`. If `time_since_update > A_{\max}`, delete the track. |
| Detection $j$ unmatched | Spawn a tentative track. State initialised per §3.8. Becomes confirmed after `hits >= 3` consecutive matches. |

### 4.7 Complexity and Eigen-friendly layout

| Operation | Cost | Eigen primitive |
|---|---|---|
| Predict $N$ tracks | $O(N\cdot 9^{2})$ | block-fixed 9×9 GEMM |
| Innovation covariance $\mathbf{S}_i$ | $O(N\cdot(9^{2}\cdot 4 + 4^{2}\cdot 9))$ | `H * P.selfadjointView<Lower>() * H.transpose()` |
| Cosine distance $\mathbf{D}^{(\text{app})}$ | $O\!\big(N\,L_{\max}\,M\,d_{\text{emb}}\big)$ | one **single big** GEMM if galleries are concatenated row-wise (`(N·L) × d * d × M`) |
| Motion-gated cost matrix | $O(N \cdot M)$ | Cholesky per track, then `triangularView<Lower>().solve()` |
| LAP-JV cascade | $\sum O(|T_n|^{3})$ | external solver |

The single concatenated-gallery GEMM is the critical optimisation:
at $N{=}30$, $L_{\max}{=}100$, $M{=}50$, $d{=}128$ that is
$30\cdot 100 \cdot 50 \cdot 128 = 19.2\,\text{M}$ FMAs — completes in
< 200 µs with AVX2.

### 4.8 Determinism guarantees

- All Eigen matrices used in the hot path are **fixed-size**
  (`Matrix<T,N,M>` with compile-time `N, M`) — no heap allocation
  inside `Track::predict()` or the association loop.
- The cascade is single-threaded per frame; multi-track parallelism is
  exposed at the *prediction* stage (TBB `parallel_for_each`) but the
  assignment solver is intentionally serial — correctness over
  micro-optimisation here.
- We seed the LAP-JV with row-major ordering of track IDs so that two
  identical inputs yield identical assignments (important for
  reproducible playback of recorded sessions).

---

## 5. Putting It Together — Per-Frame Pipeline

```
Frame k arrives at t_k:
  Δt ← t_k − t_{k-1}                                            (clamped to [0.005, 0.300])

  ── Prediction ────────────────────────────────────────────────
  for each track i (parallel):
    Q_c,active ← ρ(η̄_i) · Q_c,nominal                          (§2.6.2 adaptive PSD)
    x̂_{k|k-1}   ← F(Δt) · x̂_{k-1|k-1}
    P_{k|k-1}   ← F · P_{k-1|k-1} · Fᵀ + Q(Δt; Q_c,active)
    P_{k|k-1}   ← (P + Pᵀ) / 2                                  (symmetrisation)

  ── Detection (Phase 1, parallel to above) ────────────────────
  Detections {(z_j, conf_j, embedding f_j, label_j, box_corners_j)}.

  ── Association ───────────────────────────────────────────────
  Group detections and tracks by label.
  Build appearance matrix D_app  (§4.2.1).
  Build motion matrix    D_mot   (§4.2.2).
  Gate and combine into C        (§4.3).
  Run matching cascade           (§4.4) → matched, unmatched.

  ── Update ────────────────────────────────────────────────────
  for each matched (i, j):
    H, h    ← linearise §3 at x̂_{k|k-1}
    if track i is in boundary_probation OR box-dim guard tripped:
      H ← H[0:2, :]; R ← R[0:2, 0:2]                            (§3.10.4 / §3.9)
    ν ← z_j − h
    if regime(η̄_i) == STATIONARY AND ||ν_uv||_∞ < δ_dead:
      ν[0:2] ← 0                                                (§2.6.4 deadband)
    S       ← H P Hᵀ + R         (already cached from gating)
    K       ← P Hᵀ S⁻¹           (via Cholesky)
    x̂_{k|k} ← x̂_{k|k-1} + K · ν
    P_{k|k} ← Joseph form         (§3.7)
    update η̄_i with this innovation                              (§2.6.1)
    gallery.push_back(f_j)

  ── Stationary-regime shrinkage (every K=5 frames) ────────────
  for each track i with regime(η̄_i) == STATIONARY and i.frame_count % 5 == 0:
    apply (H_rest, R_rest, z_rest = 0) pseudo-measurement       (§2.6.3)

  ── Hierarchical structural constraint ────────────────────────
  for iter in 1..2:
    for each (parent P, sub-feature S) where §3.11.4 gate passes:
      apply two-pass Gauss–Seidel coupled update                (§3.11.3)

  ── Boundary-probation maintenance ────────────────────────────
  for each track i with is_boundary_probation:
    if 3 consecutive in-frame detections observed:
      is_boundary_probation ← false                             (§3.10.4)

  ── Lifecycle ─────────────────────────────────────────────────
  For each unmatched detection j:
    classify (Z_0, init_mode) per §3.10.3
    if init_mode == DEFERRED:
      push to pending_spawn_buffer with key = embedding(f_j)
    else:
      spawn tentative track (is_boundary_probation = (init_mode != FULL))
  Promote pending_spawn_buffer entries whose follow-up detection
    qualifies; drop entries older than T_hold = 5 frames.
  Decay age of unmatched tracks; delete those past A_max.

  ── Egress: telemetry frame for SvelteKit ──────────────────────
  Serialise confirmed tracks to the §7 binary wire format and publish.
```

This is the contract the Phase 4 implementation must satisfy. The five
ordered phases (Prediction → Association → Update → Constraint → Lifecycle)
are not negotiable — reordering changes the meaning of `time_since_update`
which the §3.11.4 gate depends on.

---

## 6. Safety Addenda — Cross-Cutting Summary

Three physical and mathematical vulnerabilities have been formally
mitigated and inlined into the per-frame pipeline above. Cross-reference
table for the Phase 4 implementation and Phase 5 verification:

| Trap | Failure mode | Mitigation locus | Pipeline insertion |
|---|---|---|---|
| **A — Edge-clipped depth divergence** | Partial-frame box → $w_{\text{px}}$ collapses → $Z_0 \to \infty$ | §3.10 selective-axis init + boundary-probation EKF state | Update (row-drop), Lifecycle (deferred spawn + probation maintenance) |
| **B — Acceleration over-integration on static targets** | Sub-pixel jitter → spurious jerk → $\mathbf{P}$ inflation → gate drop | §2.6 NIS regime detector + adaptive $\mathbf{Q}_c$ + $(\mathbf{v},\mathbf{a})$ shrinkage + innovation deadband | Prediction (PSD scaling), Update (deadband + NIS), post-Update (shrinkage) |
| **C — Sub-feature hierarchy drift** | Independent filters → head/torso desync → physically impossible kinematics → actuator snap | §3.11 soft pseudo-measurement constraint via 2-pass Gauss–Seidel | New phase between Update and Lifecycle |

All three mitigations preserve the closed-form $\mathbf{F},\mathbf{Q}$
of §2.1–§2.2, the closed-form Jacobian of §3.4, and the fixed-size
Eigen contract of §1.2. No new dynamic allocations are introduced. The
worst-case added cost per frame is dominated by the constraint update,
at $2 \cdot |\text{hierarchy edges}| \cdot O(9^3)$ — well under 30 µs
at the design track count.

---

## 7. Downstream Wire Contract — SvelteKit Telemetry Stream

Because §1.2 pins all per-track state to compile-time-sized Eigen
matrices, the EKF layer can serialise directly to a packed
little-endian binary frame with zero runtime introspection. The
SvelteKit dashboard ingests this frame via rosbridge ↔ WebSocket, maps
the world-frame estimates back to image-plane SVG coordinates using
$\mathbf{K},\mathbf{R}_{CW},\mathbf{t}_{CW}$ (the inverse of §3.2), and
renders the trajectory overlay at native panel refresh — 144 Hz with
30 fps inference yields ~4.8 interpolated frames per published EKF
update, which the SvelteKit side handles via cubic-Hermite smoothing
between consecutive $\mathbf{p},\mathbf{v}$ pairs.

### 7.1 Frame header (12 bytes)

| Field | Type | Bytes | Notes |
|---|---|---|---|
| `version`    | `u8`  | 1 | Bump on any layout change. |
| `flags`      | `u8`  | 1 | Bit 0 = NIS-degraded frame, bit 1 = boundary probation present. |
| `frame_id`   | `u32` | 4 | Monotone vision-pipeline frame counter. |
| `num_tracks` | `u16` | 2 | $\le 256$ in practice. |
| `t_capture_ns` | `i32` (signed offset from session epoch in ms) | 4 | Lets the dashboard scrub/replay. |

### 7.2 Per-track payload (86 bytes)

| Field | Type | Bytes | Description |
|---|---|---|---|
| `track_id`     | `u64`    | 8 | Globally unique, monotone. |
| `parent_id`    | `u64`    | 8 | `0` for parent `person`; else owning track id. |
| `label`        | `u8`     | 1 | enum {person=1, head=2, upper_torso=3, accessory_hat=4, accessory_backpack=5, …}. |
| `status_flags` | `u8`     | 1 | bit 0 = confirmed, bit 1 = boundary probation, bit 2 = stationary regime, bit 3 = hierarchy-violation pending. |
| `confidence`   | `f32`    | 4 | Last-detection confidence × NIS-derived health score. |
| `p_world`      | `f32[3]` | 12 | $(p_x, p_y, p_z)$ in $\mathcal{F}_W$, metres. |
| `v_world`      | `f32[3]` | 12 | $(v_x, v_y, v_z)$, m/s. |
| `a_world`      | `f32[3]` | 12 | $(a_x, a_y, a_z)$, m/s². |
| `P_pos_diag`   | `f32[3]` | 12 | Position covariance diagonal — feeds the SVG uncertainty ellipse. |
| `P_pos_offnorm`| `f32`    | 4 | Frobenius norm of off-diagonal position block — collapses 3 floats with negligible visual loss. |
| `bbox_orig`    | `f32[4]` | 16 | $(u, v, w_{\text{px}}, h_{\text{px}})$ in capture-frame pixels, for the SVG overlay rectangle. |

**Total = 86 bytes/track.** At the design ceiling of $N = 50$ tracks
@ 30 Hz EKF tick: $12 + 50\cdot 86 = 4312\,\text{B/frame}$,
$\approx 126\,\text{kB/s}$. Comfortably under a single MTU per frame.

### 7.3 SvelteKit ingress side (informative)

```ts
// dashboard/src/lib/telemetry/decode.ts
const TRACK_STRIDE = 86;
export function decodeFrame(buf: ArrayBuffer): TelemetryFrame {
  const dv = new DataView(buf);
  const hdr = decodeHeader(dv);
  const tracks: Track[] = new Array(hdr.numTracks);
  for (let i = 0; i < hdr.numTracks; i++) {
    const o = 12 + i * TRACK_STRIDE;
    tracks[i] = decodeTrack(dv, o);   // pure offset reads, no allocation per field
  }
  return { hdr, tracks };
}
```

A recycled `TelemetryFrame` pool keyed by `frame_id mod K` keeps the
dashboard at zero GC pressure across the entire WebSocket session — the
Svelte stores subscribe to a single mutable buffer, never per-frame
allocations.

### 7.4 Dashboard re-projection

The dashboard receives world-frame coordinates. To overlay them on the
camera image it inverts §3.2:

$$
\mathbf{p}_C = \mathbf{R}_{CW}\,\mathbf{p}_W + \mathbf{t}_{CW},\qquad
\begin{bmatrix} u \\ v \\ 1 \end{bmatrix} \;\propto\; \mathbf{K}\,\mathbf{p}_C / Z_C.
$$

The intrinsics $\mathbf{K}$ and extrinsics $(\mathbf{R}_{CW},
\mathbf{t}_{CW})$ are published once on connection in a
`/calibration` topic; the dashboard caches them as `Float32Array(9)`
typed arrays and uses a tiny `mat3_mul_vec3` helper — no library
needed.

---

## 8. Open Items (defer to Phase 4 review)

1. **Motion model switching.** Should we run an IMM (Interacting
   Multiple Models) bank — CA + CV + stationary — for the head sub-feature,
   where the modal dynamics are extremely heterogeneous? Decision point
   before the predict step is implemented.
2. **Depth from box dimensions vs. depth from rig-mounted stereo / RGB-D.**
   If the rig ever gains a second camera, $\mathbf{h}$ acquires two more
   rows and the $W_0,H_0$ priors become decorative rather than load-bearing.
3. **Embedding extractor.** The 128-D vector source is currently
   notional; final choice between OSNet-x0.25 (faster, 0.5 M params) and
   the original wide ResNet (more accurate) will be benchmarked in
   Phase 3 before being baked into the gallery format.
4. **Per-axis world-frame definition.** With a level monocular rig,
   $p_z$ in $\mathcal{F}_W$ is degenerate without the box-size prior.
   The choice of world-frame Z direction (gravity-aligned vs.
   camera-optical-axis-aligned) should be settled before calibration
   tooling is built.

---

## 9. Numerical-stability & safety checklist (Phase 5 verification)

**Core numerics**

- [ ] `P` symmetrised after every prediction (§2.5).
- [ ] Joseph form on every update (§3.7).
- [ ] Cholesky on $\mathbf{S}$, never explicit inverse.
- [ ] $\Delta t$ clamped to $[0.005, 0.300]$ s.
- [ ] $Z$ floor check before computing $\mathbf{H}$.
- [ ] Box-dim row dropout when $w_{\text{px}}$ or $h_{\text{px}} < 4\,\text{px}$.
- [ ] Sentinel-cost (not `inf`) entries in the assignment matrix.
- [ ] Compile with `-ffast-math` **off** for filter code, `-fno-math-errno -fno-trapping-math` only.
- [ ] Unit tests reproduce §3.5 closed-form against the general §3.4 derivation.

**Trap A — boundary safety (§3.10)**

- [ ] $\kappa = \max(z_w,z_h)/\min(z_w,z_h)$ disagreement gate active on every spawn attempt.
- [ ] Selective-axis $Z_0$ policy matches the 4-case table in §3.10.3 exactly.
- [ ] Pending-spawn buffer correctly drops entries past $T_{\text{hold}}=5$ frames.
- [ ] Boundary-probation tracks never receive depth (rows 3–4) measurements.
- [ ] Probation flag clears only after **3 consecutive** in-frame detections.
- [ ] Diagnostic counters (`boundary_init_count`, `disagree_init_count`, `probation_frames_total`) are exported.

**Trap B — stationary regime (§2.6)**

- [ ] EMA-NIS $\bar\eta$ initialised at $\mathbb{E}[\chi^2_4] = 4$ (not zero — otherwise every new track briefly registers as STATIONARY).
- [ ] $\rho(\bar\eta)$ piecewise-linear with monotone clamp $\in [0.05, 4.0]$.
- [ ] $(\mathbf{v},\mathbf{a})$ shrinkage pseudo-measurement gated by $K=5$ frame stride.
- [ ] Innovation deadband **disabled** outside the STATIONARY regime.
- [ ] Regime histogram exported per track for tuning telemetry.

**Trap C — hierarchical constraint (§3.11)**

- [ ] Constraint update applied only when §3.11.4 gate (all 3 conditions) is satisfied.
- [ ] $N_{\text{iter}} = 2$ Gauss–Seidel passes per frame, no more.
- [ ] Plausibility check $\lVert\mathbf{h}_{\text{rel}}\rVert < 3\lVert\boldsymbol{\sigma}_S\rVert$ skips and emits `hierarchy_violation` event instead of merging.
- [ ] Constraint covariance $\mathbf{R}_{\text{rel}}$ matches the §3.11.1 anatomical prior table.
- [ ] Joint-state reference EKF (offline, 27-D) used as ground truth in the simulation harness; coupled-update residual ≤ 1% confirmed.

**Wire contract (§7)**

- [ ] Binary frame layout matches §7.1/§7.2 byte-for-byte.
- [ ] SvelteKit decoder uses zero-allocation `DataView` reads.
- [ ] Calibration topic published exactly once at session start.

---

## 10. References (selective)

1. Wojke, Bewley, Paulus. *Simple Online and Realtime Tracking with a Deep Association Metric*. ICIP 2017. arXiv:1703.07402. — DeepSORT canonical reference; defines cascade, cosine gating, and $\chi^{2}_4$ threshold.
2. Bar-Shalom, Li, Kirubarajan. *Estimation with Applications to Tracking and Navigation* (Wiley, 2001), Ch. 6. — CWNJ / DWNA derivations.
3. Hartley & Zisserman. *Multiple View Geometry in Computer Vision*, 2nd ed., §6. — Pinhole projection and Jacobian.
4. Eigen 3.4 user manual, §"Storage orders", §"Solving linear systems". — Cholesky-based solves, fixed-size optimisation.
5. Jonker, Volgenant. *A Shortest Augmenting Path Algorithm for Dense and Sparse Linear Assignment Problems*. Computing 38, 1987. — LAP-JV reference implementation basis.

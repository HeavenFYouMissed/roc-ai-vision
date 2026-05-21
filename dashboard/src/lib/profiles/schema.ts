// Profile schema — §11.2 + §11.3 + §13.2

export type TargetingAnchor = 'center' | 'head' | 'upper_torso' | 'custom';

export interface ActuationBlock {
	linear_smoothing_alpha: number;
	deadzone_radius_px: number;
	fov_radius_px: number;
	velocity_curve_exponent: number;
	acceleration_curve_exponent: number;
}

export interface BgOdometryBlock {
	center_mask_w_frac: number;
	center_mask_h_frac: number;
	max_features: number;
	feature_renewal_frames: number;
}

export interface RocProfile {
	id: string;
	label: string;
	actuation: ActuationBlock;
	targeting_anchors: Record<string, TargetingAnchor>;
	bg_odometry: BgOdometryBlock;
}

export const DEFAULT_PROFILE: RocProfile = {
	id: 'default',
	label: 'Standard Kinematic Match',
	actuation: {
		linear_smoothing_alpha: 0.35,
		deadzone_radius_px: 4.0,
		fov_radius_px: 320.0,
		velocity_curve_exponent: 1.0,
		acceleration_curve_exponent: 1.0
	},
	targeting_anchors: {
		person: 'upper_torso',
		head: 'head',
		upper_torso: 'upper_torso'
	},
	bg_odometry: {
		center_mask_w_frac: 0.3,
		center_mask_h_frac: 0.3,
		max_features: 200,
		feature_renewal_frames: 30
	}
};

export function validateProfile(p: RocProfile): boolean {
	return (
		p.actuation.linear_smoothing_alpha >= 0.05 &&
		p.actuation.linear_smoothing_alpha <= 1.0 &&
		p.actuation.deadzone_radius_px >= 0 &&
		p.actuation.fov_radius_px > 0 &&
		p.bg_odometry.center_mask_w_frac > 0 &&
		p.bg_odometry.center_mask_w_frac <= 1 &&
		p.bg_odometry.max_features >= 8
	);
}

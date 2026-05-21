// =============================================================================
// roc-ai-vision · dashboard · class_colors.ts
//
// Per-class color palette for the entire dashboard. Used by:
//   · OperateCanvas Layer 2 (bbox stroke + label background pill)
//   · DetectionJsonFeed (color dot per detection card)
//   · ThresholdControls (class visibility checkbox swatches)
//   · ObjectCountBadge (per-class dot strip)
//   · HierarchyExplorerDrawer (tree row color dot)
//   · TopChrome SINK badge (sink-kind palette)
//
// Mirrors the C++ enum at core/kinematics_engine/include/roc/kinematics/types.hpp
//   Unknown          = 0
//   Person           = 1
//   Head             = 2
//   UpperTorso       = 3
//   AccessoryHat     = 4
//   AccessoryBackpack= 5
// =============================================================================

export const CLASS_NAMES: readonly string[] = [
	'Unknown',
	'Person',
	'Head',
	'UpperTorso',
	'AccessoryHat',
	'AccessoryBackpack'
] as const;

export const CLASS_SHORT_NAMES: readonly string[] = [
	'Unknown',
	'Person',
	'Head',
	'Torso',
	'Hat',
	'Backpack'
] as const;

export const CLASS_COLORS: readonly string[] = [
	'#6b6b6f', // 0 Unknown   - neutral grey
	'#00f0ff', // 1 Person    - cyan (primary subject)
	'#ff6dc7', // 2 Head      - magenta (anatomic part)
	'#34d399', // 3 UpperTorso- green (anatomic part)
	'#f5a623', // 4 AccessoryHat - orange (accessory)
	'#ff8a4c'  // 5 AccessoryBackpack - warm amber (accessory)
] as const;

// Darker variants for fills / muted backgrounds at low opacity
export const CLASS_COLORS_DIM: readonly string[] = [
	'rgba(107, 107, 111, 0.15)',
	'rgba(0, 240, 255, 0.15)',
	'rgba(255, 109, 199, 0.15)',
	'rgba(52, 211, 153, 0.15)',
	'rgba(245, 166, 35, 0.15)',
	'rgba(255, 138, 76, 0.15)'
] as const;

// Text color to use ON the colored pill background (high contrast)
export const CLASS_LABEL_TEXT: readonly string[] = [
	'#e7e5e0', // grey -> cream
	'#0a1014', // cyan -> dark
	'#1a0613', // magenta -> dark
	'#062014', // green -> dark
	'#1e1208', // orange -> dark
	'#1c0a04'  // amber -> dark
] as const;

export function classColor(label: number): string {
	return CLASS_COLORS[label] ?? CLASS_COLORS[0];
}

export function classColorDim(label: number): string {
	return CLASS_COLORS_DIM[label] ?? CLASS_COLORS_DIM[0];
}

export function classLabelText(label: number): string {
	return CLASS_LABEL_TEXT[label] ?? CLASS_LABEL_TEXT[0];
}

export function className(label: number): string {
	return CLASS_NAMES[label] ?? 'Unknown';
}

export function classShortName(label: number): string {
	return CLASS_SHORT_NAMES[label] ?? 'Unknown';
}

// -----------------------------------------------------------------------------
// SINK badge palette - mirrors core/transport OutputSinkKind enum
// -----------------------------------------------------------------------------
export const SINK_LABELS: Record<string, string> = {
	noop: 'NO-OP',
	hid: 'HID-MOUSE',
	hid_mouse: 'HID-MOUSE',
	dynamixel: 'DYNAMIXEL',
	dynamixel_servo: 'DYNAMIXEL',
	pwm_pantilt: 'PWM',
	modbus: 'MODBUS',
	logfile_csv: 'CSV-LOG'
};

export const SINK_COLORS: Record<string, string> = {
	noop: '#6b6b6f',
	hid: '#f5a623',
	hid_mouse: '#f5a623',
	dynamixel: '#ff8a4c',
	dynamixel_servo: '#ff8a4c',
	pwm_pantilt: '#a78bfa',
	modbus: '#34d399',
	logfile_csv: '#878787'
};

// -----------------------------------------------------------------------------
// Status flag display - mirrors wire_format.hpp wire_status_bits
// -----------------------------------------------------------------------------
export const STATUS_FLAG_BITS = {
	CONFIRMED: 0x01,
	BOUNDARY: 0x02,
	STATIONARY: 0x04,
	HIERARCHY_VIOLATION: 0x08
} as const;

export function statusFlagLabels(flags: number): string[] {
	const out: string[] = [];
	if (flags & STATUS_FLAG_BITS.CONFIRMED) out.push('CONFIRMED');
	if (flags & STATUS_FLAG_BITS.BOUNDARY) out.push('BOUNDARY');
	if (flags & STATUS_FLAG_BITS.STATIONARY) out.push('STATIONARY');
	if (flags & STATUS_FLAG_BITS.HIERARCHY_VIOLATION) out.push('HIERARCHY');
	return out;
}

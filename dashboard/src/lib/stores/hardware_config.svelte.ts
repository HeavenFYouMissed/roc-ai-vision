const BAUD_RATES = [115200, 230400, 460800, 921600] as const;

const SERIAL_PORTS = [
	'COM1',
	'COM3',
	'COM4',
	'/dev/ttyUSB0',
	'/dev/ttyACM0'
] as const;

class HardwareConfigStore {
	serialPort = $state<string>(SERIAL_PORTS[1]);
	baudRate = $state<number>(115200);
	cropX = $state(0);
	cropY = $state(0);
	cropWidth = $state(1920);
	cropHeight = $state(1080);
	lastDiagnosticPing = $state<string | null>(null);

	readonly baudRateOptions = BAUD_RATES;
	readonly serialPortOptions = SERIAL_PORTS;

	resetDefaults() {
		this.serialPort = SERIAL_PORTS[1];
		this.baudRate = 115200;
		this.cropX = 0;
		this.cropY = 0;
		this.cropWidth = 1920;
		this.cropHeight = 1080;
		this.lastDiagnosticPing = null;
	}

	runDiagnosticPing() {
		this.lastDiagnosticPing = `PING OK @ ${new Date().toISOString()} — ${this.serialPort} @ ${this.baudRate}`;
	}
}

export const hardwareConfig = new HardwareConfigStore();

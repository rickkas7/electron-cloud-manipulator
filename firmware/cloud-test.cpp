#include "Particle.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

const size_t READ_BUF_SIZE = 128;
char readBuf[READ_BUF_SIZE + 1];
size_t readBufOffset = 0;

bool wasCloudConnected = false;
bool wasCellularConnected = false;
bool helpPrinted = false;
char lastChar = 0;
int testEventCount = 0;

void processCommand();
void printHelp();

int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);

void setup() {
	Serial.begin(9600);

	// Register all the Tinker functions
	Particle.function("digitalread", tinkerDigitalRead);
	Particle.function("digitalwrite", tinkerDigitalWrite);

	Particle.function("analogread", tinkerAnalogRead);
	Particle.function("analogwrite", tinkerAnalogWrite);

	Particle.connect();
}

void loop() {
	if (!helpPrinted && millis() > 8000) {
		// Print help 8 seconds after boot to allow time to connect to serial
		// (you can type command before that)
		printHelp();
		helpPrinted = true;
	}

	while(Serial.available()) {
		if (readBufOffset < READ_BUF_SIZE) {
			char c = (char) Serial.read();
			if (c == '\r' || c == '\n') {
				// End of line
				if (readBufOffset > 0) {
					readBuf[readBufOffset]  = 0;
					// Have a command, process it
					Serial.println("");
					processCommand();
				}
				else {
					if (lastChar != '\r') {
						// Empty line, not CRLF, print help
						printHelp();
					}
				}
				readBufOffset = 0;
			}
			else {
				Serial.print(c);
				readBuf[readBufOffset] = lastChar = c;
				readBufOffset++;
			}
		}
		else {
			// Buffer overflow, start over
			Serial.println("line too long, ignoring");
			readBufOffset = 0;
		}
	}

	if (Cellular.ready()) {
		if (!wasCellularConnected) {
			wasCellularConnected = true;
			Serial.println("cellular connected");
		}
	}
	else {
		if (wasCellularConnected) {
			wasCellularConnected = false;
			Serial.println("cellular disconnected");
		}
	}

	if (Particle.connected()) {
		if (!wasCloudConnected) {
			wasCloudConnected = true;
			Serial.println("cloud connected");
		}
	}
	else {
		if (wasCloudConnected) {
			wasCloudConnected = false;
			Serial.println("cloud disconnected");
		}
	}
}

void processCommand() {
	char *tokens[10], *cp;
	size_t numTokens = 0;

	for(size_t ii = 0; ii < (sizeof(tokens) / sizeof(tokens[0])); ii++) {
		tokens[ii] = "";
	}

	tokens[numTokens++] = strtok(readBuf, " ");
	cp = strtok(NULL, " ");
	while(cp && numTokens < (sizeof(tokens) / sizeof(tokens[0]))) {
		tokens[numTokens++] = cp;
		cp = strtok(NULL, " ");
	}

	if (strcmp(tokens[0], "con") == 0) {
		Serial.println("* con command - connecting to cloud");
		Particle.connect();
	}
	else
	if (strcmp(tokens[0], "dis") == 0) {
		Serial.println("* dis command - disconnecting from cloud");
		Particle.disconnect();
	}
	else
	if (strcmp(tokens[0], "keep") == 0) {
		int value = atoi(tokens[1]);
		if (value < 15) {
			value = 15;
		}

		Serial.printlnf("* keep [value] - set the keepAlive value to %d", value);
		Particle.keepAlive(value);
	}
	else
	if (strcmp(tokens[0], "pub") == 0) {
		Serial.println("* pub command - publish event");

		char buf[64];
		snprintf(buf, sizeof(buf), "testEventCount=%d", testEventCount++);
		Particle.publish("cloudTest", buf, PRIVATE);
	}
	else
	if (strcmp(tokens[0], "ses") == 0) {
		Serial.println("* ses - end session (publish spark/device/session/end)");

		Particle.publish("spark/device/session/end", "", PRIVATE);
	}
	else {
		printHelp();
	}
}



void printHelp() {
	Serial.println("Enter command followed by Return. Commands are case-sensitive.");

	Serial.println("con - connect to the Particle cloud");
	Serial.println("dis - disconnect from the Particle cloud");
	Serial.println("keep [value] - set the keepAlive value");
	Serial.println("pub - publish a test event");
	Serial.println("ses - end session (publish spark/device/session/end)");

}



/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
#if Wiring_Cellular
	else if (pin.startsWith("B"))
	{
		if (pinNumber > 5) return -3;
		pinMode(pinNumber+24, INPUT_PULLDOWN);
		return digitalRead(pinNumber+24);
	}
	else if (pin.startsWith("C"))
	{
		if (pinNumber > 5) return -4;
		pinMode(pinNumber+30, INPUT_PULLDOWN);
		return digitalRead(pinNumber+30);
	}
#endif
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
#if Wiring_Cellular
	else if(command.startsWith("B"))
	{
		if (pinNumber > 5) return -4;
		pinMode(pinNumber+24, OUTPUT);
		digitalWrite(pinNumber+24, value);
		return 1;
	}
	else if(command.startsWith("C"))
	{
		if (pinNumber > 5) return -5;
		pinMode(pinNumber+30, OUTPUT);
		digitalWrite(pinNumber+30, value);
		return 1;
	}
#endif
else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(pin.startsWith("D"))
	{
		return -3;
	}
	else if (pin.startsWith("A"))
	{
		return analogRead(pinNumber+10);
	}
#if Wiring_Cellular
	else if (pin.startsWith("B"))
	{
		if (pinNumber < 2 || pinNumber > 5) return -3;
		return analogRead(pinNumber+24);
	}
#endif
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	String value = command.substring(3);

	if(command.substring(0,2) == "TX")
	{
		pinMode(TX, OUTPUT);
		analogWrite(TX, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "RX")
	{
		pinMode(RX, OUTPUT);
		analogWrite(RX, value.toInt());
		return 1;
	}

	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits

	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "TX")
	{
		pinMode(TX, OUTPUT);
		analogWrite(TX, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "RX")
	{
		pinMode(RX, OUTPUT);
		analogWrite(RX, value.toInt());
		return 1;
	}
#if Wiring_Cellular
	else if (command.startsWith("B"))
	{
		if (pinNumber > 3) return -3;
		pinMode(pinNumber+24, OUTPUT);
		analogWrite(pinNumber+24, value.toInt());
		return 1;
	}
	else if (command.startsWith("C"))
	{
		if (pinNumber < 4 || pinNumber > 5) return -4;
		pinMode(pinNumber+30, OUTPUT);
		analogWrite(pinNumber+30, value.toInt());
		return 1;
	}
#endif
else return -2;
}

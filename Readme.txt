Gainspan(GS1011MIPS) Wi-Fi Module Driver for RL78-G14
 
 
Broad Setup:
	- Initialize SPI Channel for Wi-Fi 
                - Send Turn Echo Off Command to make response handling easy
	- Try To get “OK” response to “AT” Command to confirm boot up of Module
	- On successful boot start Auto Connect setup with following commands in order
		- Disassociate (optional) (AT+WD)
		- Enable DHCP (AT+NDHCP=1)
		- Setup Auto Connect for TCP Server ( AT+NAUTO=1,1,,<port>)
		- Setup Auto Connect to connect to “wifidemo” AP (AT+WAUTO=0,wifidemo)
		- Enable Auto Connect Profile (ATC1)
		- Initiate Auto Connect (ATA)
	- On successful connection display Dynamic IP obtained from DHCP server
- Wait for client connection
	- Start Listening for messages from data

Functions:
	- Auto_Setup()
		- Setup Auto Connection using API functions and call Application after successful 
		   connection 
	- API’s for Commands
		- Echo_Disable()
			- Disable Echo from Module
		- Echo_Confirm()
			- Confirm “OK” response to AT command
		- Disassociate () 
			- Disassociate for wireless network
		- DHCP_Enable()
			- Enable DHCP server to setup a dynamic ip on connection
		- Auto_Wifi()
			- Setup Wi-Fi settings for Auto Connect
		- Auto_Network()
			- Setup Network settings for Auto Connect
		- Auto_Enable()
			- Enable Auto Connect profile
		- Auto_Start()
			- Initiate Auto_Connect Connection
	- WIFI_RecieveByte(char * buf)
		- Checks for Pin 7.7 if high means that WIFI module has data ready in buffer
		- Enables Chip-select for WIFI module
		- To flush out a byte from Module’s TX buffer an idle character is sent
- Disable Chip-select on successful reception of a byte
- Return “true” for successful reception 





	
	- Received_Char_Handle(char buf)
		- State machine to parse received bytes from a Command Response only
		- Command Response Format
			\r\n<DATA>\r\n
		- Checks character using switch case statement
			- XON (0xF5) : Ignored unless complete message has been received then successful
				             response returned
			- ‘\0’ (0x00) : Serious Error (Currently not handled)
			- ‘\n’: Check if Starting or Ending Line Feed and set appropriate bit for State
			- ‘\r’: Check if Starting or Ending Carriage Return and set appropriate bit for State
			- default : Check State bits if “Header” has been parsed then return appropriate 
     response to store byte else ignore
- Response_Handle()
		- After sending a command to the module this function listens for a response only
		  for success or failure of the command, i.e. “OK” or “ERROR”
		-  In a while loop
			- Using WIFI_ReceiveByte a byte is stored in buf
			- buf is parsed through Received_Char_Handle
			- Depending on response buf is either stored in inbuf or ignored
		- On successful completion of receiving a complete message, it is checked  if it is either “OK” or 
                                   “ERROR”
	- Data_Handle()
		- After sending a command to the module this function listens for a response  which
		   will have useful data regarding the status of the connection of the module
		-  In a while loop
			- Using WIFI_ReceiveByte a byte is stored in buf
			- buf is parsed through Received_Char_Handle
			- Depending on response buf is either stored in inbuf or ignored
		- On successfully extracting the data from the response the required information is extracted
- The two cases in this code is extraction of IP address using IP_extract   
	- Extract IP obtained from DHCP server on successful establishment of connection
	- On successful connection with Client extract IP address of Client and Connection ID
- IP_extract(uint8_t*,uint8_t type)
	- “type” defines which message is being parsed
		- type = 1 ; Extract Servers IP by extracting the string till the first colon (:)
		- type = 2 ; Extract Client IP by extracting the sub string from the 12th bit till the 24th bit
- Parse_Data(char buf)
	- Dedicated State Machine to extract data received from the client
	- Client Message Format:
 <ESC_CHAR>S<CID><DATA><ESC_CHAR>E
	- Checks character using a switch case and modifies state correspondingly:
			- XON (0xF5) : Ignored unless complete message has been received then successful
				             response returned
			- ‘\0’ (0x00) : Serious Error (Currently not handled)
			- ‘ESC_CHAR’: Check if Starting or Ending ESC character and set appropriate bit for State
			- ‘S’: Check if Starting S after Starting ESC character otherwise byte is part of the body
			- ’E’ : Check if Ending E after Ending ESC character otherwise byte is part of the body
		                - <CID> : First byte after “Header’s S” compared with extracted connection id 
			- default : Check State bits if “Header” has been parsed then return appropriate 
     response to store byte else ignore
   	- Disp_Client_Data();
		- An endless loop called upon establishing connection with client that reads data from Wi-Fi
		  module that is received from client
		- Currently displays every message received directly on the LCD screen
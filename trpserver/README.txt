TRP Protocol
------------------------------------------------
README of Server
================================================
Program Name: server.cpp
================================================
How to Run:
-----------
0. You need to have super user access to the machine to run the server.
1. Make a configuration file with any name in the same directory of the executable.
2. Insert details in the configuration file (detials given below).
3. Identify your os version i.e. 32 bit or 64 bit. Use 'uname -m' command for this. If it returns 
   'x86_64', you should use server_64 binary. In other cases you should use server_32 binary.
4. Run the program $sudo ./server_<os bit version> <name of the configuration file made by you>
e.g. $sudo ./server_32 config.txt  OR $sudo ./server_64 config.txt

=================================================


Insert Details in config.txt:
-----------------------------
$$ Config file should be written with care. Follow the given file. Some detials to remember are:
	1. Every line contain a single configuration property.
	2. No line should contain more than one equal sign(=) 
	3. After # sign, the rest part of a line will be treated as comment.
	4. First write the property name, then equal sign (=) at the end vlaues.
SOURCE_ADDRESS : IP address of the machine in which server is running. e.g. 10.129.75.101
DESTINATION_ADDRESS : IP address of the client. e.g. 10.129.76.101
LOCAL_INTERFACE : Local network interface name e.g. eth0
WINDOW_SIZE_START : This is an integer value. Size of the window of burst at the start of the 
			program. e.g 4. This means how much packets need to send in a window at 
			the start of the program.
VARIABLE_WINDOW : 0 or 1. If it is 0 then fixed window size will be used, if it is 1 then window size
			will be increased according to protocol
LOG_FILE_NAME : File name that will used to write log
LAST_PACKET_NUMBER : How many packets to send
DROP : Which packets will be dropped. This will be written as comma separated e.g. DROP = 10,18,32,50
	Note that this numbers may not same as the packet number written in the packet. Rather if for
	any value 'x' in DROP the x-th packet with 128 byte(Size mismatch-ed packet will be dropped 
	before this check applied ) will be dropped.
ACK_AFTER : A single integer 'z' value >0 . ACK will be send in every z-th packet interval.
==================================================


Explanation of Log:
-------------------
Log has four field in every row. 
First field: (Event no) this means when ever a packet reaches in the server end an event is created. 
		So this field calculates the event for which log is created. Note that, this number 
		is not same as the packet number or may not even be same as the number of proper
		packet accepted by the server.
Second field: (Event Name) this field contains event names associated with a particular event. The all
		possible events names are mentioned below with description.
Third field: (Expected Value) if not mentiond otherwise this field will contains expected value during
		an error event occurance. Some few events where this field is not required and used for
		other purpose. e.g. For ACK_SEND event this field will contain the value of the packet
		number for which ACK is send to the client.
Fourth field: (Actual Field) During any erronious event this field contains the actual value that received
		in the TRP packet received from client. e.g. A packet with size 129 byte recived from the
		client. So this field will contain the value 129 while the event name will be SIZE_MISMATCH.
		


----------------------------------------------------
List of event names:
--------------------
$$ The list contains some description. expected-field means third column in the log while the actual-field
contains the fourth column of the log.
$$ -1 value in a field should be taken as Not Applicable.
$$ Initial program start and socket creation are taken as event number 0.
--------------
START : Start of the program
SOCKFAIL : Error in creating socket
SOCKSUCCESS : Successfully created socket
SETSOCKOPTFAIL : Setsocktopt() function call fail
SETSOCKOPTOK: setsockopt() function call ok
NOTDATA    :  flag field is not zero so the recived packet is not data packet
DROP_EXTRA : Burst completed but other packets coming within 1 sec window so dropped
WINDOW_COMPLETE : All packets of the window received properly. expected field contains the max_burst_window value
WINDOW_UPDATE : Burst window size updated. expected-field : old value, actual-field: new value
SIZE_MISMATCH  : size is not 128 byte
DROP_SIM : Dropped by network , expected-field : packet number of the dropped packet, actual-field: burst seq of the dropped packet
PKNO_MISMATCH : Packet no. not mathced with expected
RECV_OK : Packet received and accepted successfully. Attention: here expected field contain packet number, actual field contains burst_seq
ACK_SEND: Ack send.  expected-field: contains the packet number send in ack
MAX_BURST_MISMATCH: Max burst window size is malformed in the TRP packet sent by the client
FINISH: Test finishes i.e 60 packets received. Summary will be added in the end of log.	
======================================================			

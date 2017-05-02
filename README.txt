Author: Karl Pircher
Student ID: 26898543

---------
Compiling
---------
Makefile
	Example: make
		This will clean previous executables and compile into object code
		file and then link them to executables.

----------------------
Command Line Arguments
----------------------

Hide Message
	Example: ./hide infile.ppm outfile_name

	infile.ppm
		(required) Specifies the ppm image that the message will be hidden 
		within. Input file must be P6 format and have a maximum channel
		value of 255.
		
	outfile_name
		(required) Specifies the name that the output ppm image containing 
		the hidden message will take.

Unhide Message
	Example: ./unhide hidden_msg_img.ppm
		(required) Specifies the ppm image that contains a hidden message.
		Must be P6 format and have a maximum channel value of 255.
		
-------------
Functionality
-------------

The hide function can hide a string in a ppm image.

The unhide function can extract a string that is hidden within a ppm image.

-----------
Limitations
-----------
The hide function is able to hide a message that is at most, three times the 
number of pixels in the input image and the colour channel has a maximum
value of 255. The ppm image must be P6 format as the program uses bit 
manipulation to hide the message in the image. This is implemented by changing 
the least significant bit of a colour channel to the value of the message bit.

The unhide function is able to extract the hidden message, so long as the 
input image has a maximum colour channel value of 255. The ppm image must also 
be P6 format due to bitwise operations being used to recover the hidden message.








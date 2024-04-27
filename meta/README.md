# Meta Directory

This directory will contain all "well-known" descriptions. E.g. service interfaces which will be required often will get
a well known interface description here.

Drivers can then implement this service interface in order to provide a connection to the xbot framework.

E.g. the SimpleMotorService interface might receive a speed signal and output the motor speed. Any specialized motor
drivers can be created implementing this interface.
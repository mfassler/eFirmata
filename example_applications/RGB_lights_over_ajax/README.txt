
This is an example web application to control some lights.  

The idea is that you use PWM channels 0, 1 and 2 to control some 
red, green and blue lights.  From a web-page, you've got a 
color-wheel that you can click on to see real-time updates to the 
color and brightness.  


Setup:

                   Web Browser
               (eg, a smartphone)
                       |
                       |
                   Web Server
                       |
                       |
                    eFirmata
                       |
               PWM channels 0, 1, 2
        (controlling Red, Green, Blue lights)


The smartphone makes AJAX calls to the web server.  On the server, 
a PHP script sends UDP packes to the eFirmata.

The PHP script needs to know the IP address and port number of the 
eFirmata you want to send to.  

If you're in the mood for, say, purple light, you just touch the 
purple part of your screen.  


(In principle, you could build an app directly for the smartphone 
to send UDP packets directly across the network.)

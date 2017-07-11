Garage door auto-closer

Have you ever left your garage door open by mistake, and later found someone
you don't know in there? You can buy an automatic garage door closer like
https://www.amazon.com/Garage-Butler-Automatic-Door-Closer/dp/B002EJULB8,
...or you can more fun by building one yourself!

This project uses an Arduino Nano to detect when a garage door has been 
open for too long, and if so, simulates a press to the garage door's 
"close" pushbutton. 

The inputs are two magnetic reed sensors: one reports if the door is 
completely open, and one reports if the door is completely closed. 
The output is a relay contact closure that is wired in parallel to 
the garage door pushbutton.

As Arduino projects go, I would rate this as "beginner level".

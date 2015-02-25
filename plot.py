#!/usr/bin/python
import serial
import numpy as np
import gtk

from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

port = "COM5"
	
def quit_app(event):
	ser.close()
	quit()

try:
	# It seems that sometimes the port doesn't work unless 
	# you open it first with one speed, then change it to the correct value
	ser = serial.Serial(port, 2600, timeout = 0.050)
	ser.flushInput()
	ser.baudrate = 9600
except:
	print "Opening serial port failed"
	quit()

# Create a window to put the plot in
win = gtk.Window()
# Connect the destroy signal (clicking the x in the corner)
win.connect("destroy", quit_app)
win.set_default_size(400, 300)

yvals = np.zeros(50) #array to hold last 50 measurements
times = np.arange(0, 50, 1.0) # 50 from 0 to 49.

#create a plot:
fig = Figure()
ax = fig.add_subplot(111, xlabel = "Time Step", ylabel = "Distance (cm)")
ax.set_ylim(-200, 200) # set limits of y axis.

canvas = FigureCanvas(fig) # put the plot onto a canvas
win.add(canvas) # put the canvas in the window

# Show the window
win.show_all()
win.set_title("ready to receive data");

line, = ax.plot(times,yvals)
ser.flushInput()

while (1): # loop forever
	data = ser.read(4) # read data off serial input
	if len(data) > 3:
		yvals = np.roll(yvals, -1) # scoot over
		yvals[49] = np.fromstring(data, dtype="<f") # parse float
		line.set_ydata(yvals) # draw the line
		fig.canvas.draw() # update the canvas
		win.set_title("Dist: " + str(yvals[49]) + " cm")
	while gtk.events_pending():	# makes sure the GUI updates
		gtk.main_iteration()
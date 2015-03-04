#!/usr/bin/python
import serial
import numpy as np
import gtk
import math

from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

port = "COM5"
size = 20
	
def quit_app(event):
	ser.close()
	quit()

try:
	# It seems that sometimes the port doesn't work unless 
	# you open it first with one speed, then change it to the correct value
	ser = serial.Serial(port, 2600, timeout = 0.5)
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

vx = np.zeros(size) #array to hold last 50 measurements
vy = np.zeros(size) # 50 from 0 to 49.
cy = 0
cx = 0

#create a plot:
fig = Figure()
ax = fig.add_subplot(111, xlabel = "Time Step", ylabel = "Distance (cm)")
ax.set_ylim(0, 50) # set limits of y axis.
ax.set_xlim(-50, 50) # set limits of y axis.

canvas = FigureCanvas(fig) # put the plot onto a canvas
win.add(canvas) # put the canvas in the window

# Show the window
win.show_all()
win.set_title("ready to receive data");

line, = ax.plot(vx,vy,'k.')
cline, = ax.plot(cx,cy,'ms')
ser.flushInput()

while (1): # loop forever
	raw = ser.read(8) # read data off serial input
	if len(raw) > 7:
		vx = np.roll(vx, -1) # scoot over
		vy = np.roll(vy, -1) # scoot over
		data = np.fromstring(raw, dtype="<f", count=2) # parse float
		vx[size - 1] = cx
		vy[size - 1] = cy
		cx = data[0]
		cy = math.sqrt(abs(data[1]))

		line.set_xdata(vx) # draw the line
		line.set_ydata(vy) # draw the line
		cline.set_xdata(cx)
		cline.set_ydata(cy)

		fig.canvas.draw() # update the canvas
		win.set_title("Dist: " + str(cy) + " cm")
	while gtk.events_pending():	# makes sure the GUI updates
		gtk.main_iteration()
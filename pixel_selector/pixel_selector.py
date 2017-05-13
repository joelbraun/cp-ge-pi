from Tkinter import *
from tkFileDialog import askopenfilename
from PIL import Image, ImageTk


class Clicker():
    def __init__(self, num_spots):
        self.point_ctr = 0
        self.num_parking_spots = num_spots
        self.id_current_spot = 0

    def click(self, event):
        if self.point_ctr == self.id_current_spot == 0:
            print(str(self.id_current_spot)),
            # outputting x and y coords to console
        print(str(event.x) + " " + str(event.y)),
        self.point_ctr = (self.point_ctr + 1) % 4

        if self.point_ctr == 0:
            print("")
            self.id_current_spot += 1
        if self.id_current_spot == self.num_parking_spots and self.point_ctr == 0:
            exit(0)

if __name__ == "__main__":
    root = Tk()
    #setting up a tkinter canvas with scrollbars
    frame = Frame(root, bd=2, relief=SUNKEN)
    frame.grid_rowconfigure(0, weight=1)
    frame.grid_columnconfigure(0, weight=1)
    xscroll = Scrollbar(frame, orient=HORIZONTAL)
    xscroll.grid(row=1, column=0, sticky=E+W)
    yscroll = Scrollbar(frame)
    yscroll.grid(row=0, column=1, sticky=N+S)
    canvas = Canvas(frame, bd=0, xscrollcommand=xscroll.set, yscrollcommand=yscroll.set)
    canvas.grid(row=0, column=0, sticky=N+S+E+W)
    xscroll.config(command=canvas.xview)
    yscroll.config(command=canvas.yview)
    frame.pack(fill=BOTH,expand=1)

    #adding the image
    File = askopenfilename(parent=root, initialdir="/",title='Choose an image.')
    img = ImageTk.PhotoImage(Image.open(File))
    canvas.create_image(0,0,image=img,anchor="nw")
    canvas.config(scrollregion=canvas.bbox(ALL))
    clicker = Clicker(int(sys.argv[1]))

    #function to be called when mouse is clicked
    def printcoords(event):
        clicker.click(event)

    #mouseclick event
    canvas.bind("<Button 1>", printcoords)

    root.mainloop()

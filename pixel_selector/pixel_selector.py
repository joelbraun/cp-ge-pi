import sys
import pymouse
import re
from pymouse import PyMouseEvent


class Clicker(PyMouseEvent):
    def __init__(self, num_parking_spots):
        PyMouseEvent.__init__(self)
        self.point_ctr = 0
        self.line_ctr = 0
        self.num_parking_spots = num_parking_spots
        self.mouse = pymouse.PyMouse()
        sys.stdout.write(str(self.line_ctr))


    def click(self, x, y, button, press):
        if button == 1:
            if press:
                pos_str = re.sub('[(,)]', '', str(self.mouse.position()))
                sys.stdout.write(" " + pos_str)
                self.point_ctr = (self.point_ctr + 1) % 4
                if self.point_ctr == 0:
                    sys.stdout.write('\n')
                    self.line_ctr += 1
                    if self.line_ctr == self.num_parking_spots:
                        exit(0)
                    sys.stdout.write(str(self.line_ctr))

        else:  # Exit if any other mouse button used
            self.stop()

#C = Clicker(int(sys.argv[1]))
C = Clicker(2)
C.run()


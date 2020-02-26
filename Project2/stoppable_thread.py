'''
filename: stoppable_thread.py
description: this file creates a class that inherits from thread. The subclass has methods
             that allow user to cancel a thread from outside the thread.
sources: https://stackoverflow.com/questions/323972/is-there-any-way-to-kill-a-thread
'''

import threading
import sys

class StoppableListenThread(threading.Thread):
    """Thread class with a stop() method. The thread itself has to check
    regularly for the stopped() condition."""

    def __init__(self, target, name, *args, **kwargs):
        super(StoppableListenThread, self).__init__(None, target, name, *args, **kwargs)
        self._stop_event = threading.Event()
        self._exit_thread()

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()
    
    def assign_sock(self, sock):
        self._sock = sock

    def _exit_thread(self):
        if self.stopped():
            self._sock.close()
            self._sock = None
            sys.exit(0)
        else:
            t = threading.Timer(0.5, self._exit_thread)
            t.start()


import os
import sys

PROFILE = "release"
sys.path.insert(0, os.path.join(f"cmake-build-{PROFILE}", "lib"))

import csgopy


class Client(csgopy.Client):
    def on_server_class_creation(self, server_class):
        print(server_class)

    def on_game_event(self, game_event):
        print(game_event.type, game_event.keys())


client = Client(os.path.join("demos", "eg-black-vs-kari-m1-mirage.dem"))
client.parse()

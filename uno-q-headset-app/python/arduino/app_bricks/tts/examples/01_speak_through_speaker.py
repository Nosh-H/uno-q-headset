# SPDX-FileCopyrightText: Copyright (C) Arduino s.r.l. and/or its affiliated companies
#
# SPDX-License-Identifier: MPL-2.0

# EXAMPLE_NAME = "Speak text through a speaker"

from python.arduino.app_bricks.tts import TextToSpeech
from python.arduino.app_utils import App
import time


tts = TextToSpeech()


def runner():
    tts.speak("Hello world, Arduino!")
    time.sleep(5)  # Wait for the speech to finish before ending the app


App.run(user_loop=runner)

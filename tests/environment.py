# -*- coding: UTF-8 -*-

from time import sleep
from dogtail.utils import isA11yEnabled, enableA11y
if not isA11yEnabled():
    enableA11y(True)

from common_steps import App, cleanup
from dogtail.config import config
from os import system, makedirs, path
from shutil import copyfile
from gi.repository import GLib

def before_all(context):
    """Setup stuff
    Being executed before all features
    """
    try:
        # Skip dogtail actions to print to stdout
        config.logDebugToStdOut = False
        config.typingDelay = 0.2

        # Get XDG dirs
        datadirs = GLib.get_system_data_dirs()
        tmpdir = GLib.get_tmp_dir()
        homedir = GLib.get_home_dir()
        datadir = 0
        for ddir in datadirs:
            if path.exists(ddir + "/icons/Adwaita/256x256/devices/"):
                datadir = ddir

        # Set up test data directories, use Adwaita icons as test data
        makedirs(homedir + '/test_photos')
        system("find " + datadir + "/icons/Adwaita/256x256/devices/ -iname '*.png' -exec cp {} " + homedir + "/test_photos/ \;")
        system("tracker-sandbox.py -i " + tmpdir + "/tracker -c " + homedir + "/test_photos -u");

        context.app_class = App('tracker-sandbox.py -i ' + tmpdir + '/tracker/ -s', a11yAppName='gnome-photos')

        context.screenshot_counter = 0
        context.save_screenshots = False
    except Exception as e:
        print("Error in before_all: %s" % e.message)

def after_all(context):
    """
    Remove all test data directories
    """
    system("rm -rf $HOME/test_photos")
    system("rm -rf /tmp/tracker")

def before_tag(context, tag):
    try:
        # Copy screenshots
        if 'screenshot' in tag:
            context.save_screenshots = True
            context.screenshot_dir = "../eog_screenshots"
            makedirs(context.screenshot_dir)
    except Exception as e:
        print("Error in before_tag: %s" % str(e))


def after_step(context, step):
    try:
        if hasattr(context, "embed"):
            # Embed screenshot if HTML report is used
            system("dbus-send --session --type=method_call " +
                   "--dest='org.gnome.Shell.Screenshot' " +
                   "'/org/gnome/Shell/Screenshot' " +
                   "org.gnome.Shell.Screenshot.Screenshot " +
                   "boolean:false boolean:false string:/tmp/screenshot.png")
            if context.save_screenshots:
                # Don't embed screenshot if this is a screenshot tour page
                name = "%s/screenshot_%s_%02d.png" % (
                    context.screenshot_dir, context.current_locale, context.screenshot_counter)

                copyfile("/tmp/screenshot.png", name)
                context.screenshot_counter += 1
            else:
                context.embed('image/png', open("/tmp/screenshot.png", 'r').read())
    except Exception as e:
        print("Error in after_step: %s" % str(e))


def before_scenario(context, scenario):
    """ Cleanup previous settings and make sure we have test files in /tmp/photos """
    try:
        cleanup()
    except Exception as e:
        print("Error in before_scenario: %s" % e.message)


def after_scenario(context, scenario):
    """Teardown for each scenario
    Kill gnome-photos (in order to make this reliable we send sigkill)
    """
    try:
        # Stop gnome-photos
        context.app_class.kill()
        sleep(1)
    except Exception as e:
        # Stupid behave simply crashes in case exception has occurred
        print("Error in after_scenario: %s" % e.message)

from behave import step, then

from common_steps import *
from time import sleep
from dogtail.rawinput import keyCombo
from subprocess import Popen, PIPE
from dogtail import i18n

@step(u'Select first image')
def select_an_image(context):
    context.app.child(translate('Select Items')).click()
    items = context.app.child(roleName='panel', name=translate('Recent'))[0][0][0]
    items[0].click()
    sleep(1)

@then(u'all selection toolbar buttons are sensitive')
def all_selection_toolbar_buttons_sensitive(context):
    sleep(0.5)
    assert context.app.child(translate('Add/Remove from favorites')).sensitive
    assert context.app.child(translate('Open with')).sensitive
    assert context.app.child(translate('Print')).sensitive
    assert context.app.child(translate('Delete')).sensitive
    assert context.app.child(translate('Add to Album')).sensitive
    assert context.app.child(translate('Properties')).sensitive
    sleep(0.5)

@then(u'Cancel button is sensitive')
def cancel_button_is_sensitive(context):
    assert context.app.child(translate("Cancel Selection")).sensitive

@then(u'Search button is sensitive')
def search_button_is_sensitive(context):
    assert context.app.child(translate("Search")).sensitive
    
@then(u'{button} button is {negative:w} sensitive')
def button_is_sensitive(context, button, negative=None):
    actual = context.app.child(translate(button)).sensitive
    assert actual == (negative is None)

@step(u'Select first three images')
def select_multiple_images(context):
    context.app.child(translate('Select Items')).click()
    items = context.app.child(roleName='panel', name=translate('Recent'))[0][0][0]
    items[0].click()
    items[1].click()
    items[2].click()

@step(u'Mark multiple images as favorites')
def mark_multiple_images_as_favorites(context):
    context.execute_steps(u"""
    * Select first three images
    """)
    context.app.child(translate('Add/Remove from favorites')).click()
    sleep(2)

@then(u'multiple images are added to favorites')
def multiple_images_are_added_to_favorites(context):
    # go to favorites view
    context.app.child(roleName='radio button', name=translate('Favorites')).click()

    # check number of favorite items marked
    items = context.app.child(roleName='panel', name=translate('Favorites'))[0][0][0]

    # we marked 3 items as favorites above
    assert len(items) == 3

    # go back to recent view
    context.app.child(translate('Recent')).click()


@step(u'Delete third image')
def delete_an_image(context):
    items = context.app.child(roleName='panel', name=translate('Recent'))[0][0][0]
    context.no_of_images = len(items)
    context.app.child(translate('Select Items')).click()
    items[2].click()
    context.app.child(roleName='push button', name=translate('Delete')).click()

@then(u'one image is deleted from overview')
def one_image_is_deleted_from_overview(context):
    items = context.app.child(roleName='panel', name=translate('Recent'))[0][0][0]
    assert context.no_of_images - 1 == len(items)

@step(u'Click "{button}" button')
def click_button(context, button):
    if button == "Favorite toggle":
        context.favorite_state = context.app.child(translate('Favorite toggle')).checked
    context.app.child(name=translate(button)).click()

@step(u'Wait {nos:d} second')
def wait_a_second(context, nos):
    sleep(nos)

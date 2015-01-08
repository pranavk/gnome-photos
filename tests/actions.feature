Feature: Smoke tests

  Background:
    * Make sure that Photos is running

  Scenario: Basic operations
    * Wait 1 second
    * Select first image
    Then all selection toolbar buttons are sensitive
    And Cancel button is sensitive
    And Search button is sensitive
    * Click "Cancel Selection" button
    * Select first three images
    Then Print button is not sensitive
    And Properties button is not sensitive
    * Click "Cancel Selection" button
    * Mark multiple images as favorites
    Then multiple images are added to favorites
    * Delete third image
    Then one image is deleted from overview

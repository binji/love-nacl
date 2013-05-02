"use strict";

function $(selector) {
  return document.querySelector(selector);
}

var enabledMessage = '<span class="enabled">On</span>, and ready to LÖVE!';
var disabledMessage = '<span class="disabled">Off</span>... no LÖVE for me...';

function setEnabledText(enabled) {
  $('#enableStatus').innerHTML = enabled ?  enabledMessage : disabledMessage;
  $('#enableLink').textContent = enabled ? 'disable' : 'enable';
}

function onEnableClicked(e) {
  chrome.runtime.getBackgroundPage(function (backgroundWindow) {
    setEnabledText(backgroundWindow.toggleEnabled());
  });
}

function onTestClicked(e) {
  chrome.tabs.create({url: 'test.html'});
  window.close();
}

function onLoadFileClicked(e) {
  chrome.tabs.create({url: 'index.html'});
  window.close();
}

document.addEventListener('DOMContentLoaded', function () {
  chrome.runtime.getBackgroundPage(function (backgroundWindow) {
    setEnabledText(backgroundWindow.enabled);

    // Hide the update badge when the popup is displayed.
    backgroundWindow.hideBadge();
  });

  $('#enableLink').addEventListener('click', onEnableClicked, false);
  $('#testLink').addEventListener('click', onTestClicked, false);
  $('#loadFileLink').addEventListener('click', onLoadFileClicked, false);
});

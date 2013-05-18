if (document.embeds[0] &&
    document.embeds[0].name === 'plugin' &&
    document.embeds[0].type === 'application/x-love-game') {
  if (!document.body.getAttribute('application_x_love_game_injected')) {
    document.body.setAttribute('application_x_love_game_injected', 'true');
    chrome.runtime.sendMessage('mompnkcmpbopandjnddeecgeeojegohc',
                               'injected');
  }
}

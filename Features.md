<font size='3'>
We'll use this page to document all the features that will be put into the client. We can sort them into three groups: Beta, Release, Post-Release. We can use the strike-through tag (<code>&lt;s&gt;&lt;/s&gt;</code>) to indicate finished features as they are implemented.<br>
<br>
<h3>Beta</h3>

This will hold the core features that are typical of an IRC client. There won't be many features that distinguish this client from other IRC clients.<br>
<br>
<ul><li><s>basic UI to connect to a server by specifying server and port</s>
<ul><li><s>the UI will be embedded into the server status window when disconnected</s>
</li><li><s>when connected, there will be a small button at the top (just inside the output control) that you can click which will pop up the UI to connect to another</s>
</li><li><s>connect button should be somewhat transparent</s>
</li></ul></li><li>basic UI to manage configuration settings (a list of settings is needed)<br>
<ul><li>a better UI for the options dialog (and support for plugin options) can be decided on at a later stage<br>
</li></ul></li><li>UI to access channel settings, such as topic, modes, bans, excepts, invites<br>
<ul><li>a simple dialog would do, but perhaps it's better to go with the theme of the transparent button in the top-left corner and the embedded UI<br>
</li></ul></li><li>full implementation of the IRC protocol (including CTCP)<br>
</li><li><s>debug messages window, which prints out all raw messages to/from server</s>
</li><li>chat output control<br>
<ul><li><s>needs to be custom-made</s>
</li><li>dynamic color changes<br>
<ul><li>when the user changes color options, all existing text is changed<br>
</li></ul></li><li>old chat text is clipped after a certain number of lines of chat<br>
</li><li>URL highlighting (or clickable URLs)<br>
</li><li>channel-specific chat output<br>
<ul><li><s>nick highlighting: your nick appears in bold or the entire line is a certain color</s>
</li><li>other nicks can be right-clicked, and a menu UI will be available with some options<br>
<ul><li>msg/notice<br>
</li><li>whois<br>
</li><li>ctcp options<br>
</li></ul></li><li><s>nick completion</s>
<ul><li><s>press tab and the client tries to auto-complete the nickname you are typing</s>
</li></ul></li></ul></li></ul></li><li>chat logging</li></ul>


<h3>Release</h3>

This is the set of features that will distinguish the client from other IRC clients (well, a basic IRC client anyway).<br>
<br>
<ul><li>aliases<br>
<ul><li>basic implementation of defining shorthand commands to execute a set of much longer commands<br>
</li><li>might want to include some basic error checking for invalid commands (ie too few parameters specified)<br>
</li></ul></li><li>search bar<br>
<ul><li>similar to firefox's in that it searches while the user types<br>
</li><li>similar to chrome's in that it marks all the found results in the scrollbar (with small lines)<br>
</li></ul></li><li>smart nick completion<br>
<ul><li>it assigns higher precedence to nicks that have spoken recently, so when performing nick completion with the tab button, you're more likely to get a nickname that you want; after the precedence, it is still ordered alphabetically<br>
</li></ul></li><li>plugin API and scripting language integration (will need to be defined in much more detail later)<br>
<ul><li><s>event system, and hooking mechanism where plugins can have their functions called when a certain event takes place (either before or after the event)</s>
</li><li>capability to manipulate any windows and containers (size and position)<br>
</li><li>capability to execute any commands that are possible via the input control (both general client commands and IRC commands)<br>
<ul><li>we can probably just create functions that are exposed to the plugin API and which are also used by the chat input control for commands<br>
</li></ul></li></ul></li><li>channel list window<br>
<ul><li>mostly completed already; it just needs a redesign of the actual list control so it can support faster display (for lists on large networks, like freenode)<br>
</li></ul></li><li>shape the UI for config settings and server connection to make it more user-friendly</li></ul>

<h3>Post-Release</h3>

These are features that are nice to have, but aren't necessarily vital to release. In fact, they may be implemented through the plugin API instead of becoming a part of the core client.<br>
<br>
<ul><li>some form of indication for the user that the chat output scrollbar is not scrolled down all the way<br>
</li><li>proxy tunneling<br>
<ul><li>zstars requested this, and I don't know much about it, but we can <a href='http://es.wikipedia.org/wiki/SOCKS'>do some research</a>
</li></ul></li><li>capability to apply custom skins
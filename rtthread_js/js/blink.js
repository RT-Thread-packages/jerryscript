// blink.js
function blink (value)
{
  // userLed, native c function api
  userLed (0, value);
}

print ("blink js OK");

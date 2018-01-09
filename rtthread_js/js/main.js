// main.js
function loop(cnt) {
    print("in main.js loop cnt is ", cnt);
    var value = 0;

    for (var i = 0; i < cnt; i++) {
        if (i % 2 == 0) {
            value = 1;
        }
        else {
            value = 0;
        }
        blink(value);
    }

}

loop(10);

print("main js OK");

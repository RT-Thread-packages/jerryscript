
print("## start pm class js");

/* for(var i in this)
{
    print(this, " i ", i, "this[i]", this[i]);
} */
print("## end for in this");

// var pm_no_param = pmClass();
var pm_param = pmClass(1111);

// print("## pm_no_param.getId(): ", pm_no_param.getId());
print("## pm_param.getId(): ", pm_param.getId());

// pm_no_param.setId(1024);
// print("## pm_no_param.getId(): ", pm_no_param.getId());
print("## pm_param.getId(): ", pm_param.getId());

pm_param.setId(2048);
// print("## pm_no_param.getId(): ", pm_no_param.getId());
print("## pm_param.getId(): ", pm_param.getId());


function pm_extension() {
    this.tag = "extension";
}

pm_extension.prototype.rt_kprintf = function (str) {
    print(str);
};

var pm_ext = new pm_extension();
pm_ext.rt_kprintf("HelloWorld");
pm_ext.rt_kprintf(pm_ext.tag);

print("in js before Window()");

Window({
    data: {
        motto: '简易计算器☞',
        userInfo: {},
        defaultSize: 'default',
        disabled: false,
        iconType: 'info_circle'
    },
    //事件处理函数
    onButton: function () {
        print ("[js] in onButton function");
    }
});

print("in js End Window()");

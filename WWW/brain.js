function Observer() {
    this.handlers = [];  // observers
}

Observer.prototype = {

    subscribe: function (fn) {
        this.handlers.push(fn);
    },

    unsubscribe: function (fn) {
        this.handlers = this.handlers.filter(
            function (item) {
                if (item !== fn) {
                    return item;
                }
            }
        );
    },

    fire: function (o, thisObj) {
        var scope = thisObj || window;
        this.handlers.forEach(function (item) {
            item.call(scope, o);
        });
    }
}

var scaleReady = new Observer();

function reconnect(){
    return new WebSocket("ws://inuits-vaha.local:80");
}
var tm;
function checkAlive(ws){
    if(ws.readyState == 1)
    ws.send('__ping__');
        tm = setTimeout(function () {

           /// ---connection closed ///
            console.log('Pong took too long')
           if(ws.readyState == 1) ws.close();
           $('#disconnected').removeClass('hide');
        $('#connected').addClass('hide');
        console.log('Unexpected disconnect.');


    }, 5000);
    
}

function pong(){
    clearTimeout(tm);
}

function scaleFound(){
    var scaleSocket = reconnect();

    scaleSocket.onopen = ()=>{
        $('#disconnected').addClass('hide');
        $('#connected').removeClass('hide');
        console.log('Successfully connected.');
        al = setInterval(checkAlive, 7000, scaleSocket);
    };
    scaleSocket.onclose = ()=>{
        $('#disconnected').removeClass('hide');
        $('#connected').addClass('hide');
        clearInterval(al);
        findScale();
    };
    scaleSocket.onerror = ()=>{
        $('#disconnected').removeClass('hide');
        $('#connected').addClass('hide');
        console.log('Unexpected error.');
        clearInterval(al);
        if(ws.readyState != 1)
        findScale();
    };
    scaleSocket.onmessage = (mes)=>{
        if(mes.data === '__pong__') {
            pong();
            return;
        }
        let payload = parseFloat(mes.data);
        if(payload < 0) payload = 0;
        $('#weight').text(payload);
        let fl = payload;
        if((fl > 15.0 && fl < 18.0) || (fl < 25.0 && fl > 22.0)){
            $('#weight').addClass('orange-text').removeClass('red-text').removeClass('green-text');
        }
        else if(fl <= 22.0 && fl >= 18.0){
            $('#weight').removeClass('orange-text').removeClass('red-text').addClass('green-text');
        }
        else{
            $('#weight').removeClass('orange-text').addClass('red-text').removeClass('green-text');
        }
    };
}

function timeout(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function findScale(){
    try{
    await $.ajax({
        url: "http://inuits-vaha.local:8080/amIScale",
    }).done((data)=>{
        scaleReady.fire('Scale found');
    }).fail(async function(){
        await timeout(1000);
        await findScale();
    });
}
catch{}
}

$(document).ready(async function(){
    scaleReady.subscribe(scaleFound);
    await findScale();
    $('#weight').change(function(){
        let current = $('#weight').text();
        let fl = parseFloat(current);
        if(fl > 18.0 || (fl < 25.0 && fl > 22.0)){
            $('#weight').addClass('orange-text').removeClass('red-text').removeClass('green-text');
        }
        else if(fl <= 22.0 || fl >= 18.0){
            $('#weight').removeClass('orange-text').removeClass('red-text').addClass('green-text');
        }
        else{
            $('#weight').removeClass('orange-text').addClass('red-text').removeClass('green-text');
        }
    });
});



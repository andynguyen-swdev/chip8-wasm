let api, video;

Module.onRuntimeInitialized = async _ => {
    api = {
        getVideoPointer: Module.cwrap('getVideoPointer', 'number', []),
        allocateROM: Module.cwrap('allocateROM', 'number', ['number']),
        loadROM: Module.cwrap('loadROM', 'void', []),
        getROMPointer: Module.cwrap('getROMPointer', 'number', []),
        setDelay: Module.cwrap('setDelay', 'void', ['number']),
        start: Module.cwrap('start', 'void', []),
        stop: Module.cwrap('stop', 'void', []),
        resetState: Module.cwrap('resetState', 'void', []),
    };

    api.setDelay(3);

    document.getElementById("input_rom").addEventListener("change", function (e) {
        const file = e.target.files[0];
        const reader = new FileReader();
        reader.readAsArrayBuffer(file);
        reader.onload = function (ev) {
            const romBuffer = ev.target.result;
            const romPointer = api.allocateROM(romBuffer.byteLength);
            // noinspection JSCheckFunctionSignatures
            Module.HEAPU8.set(new Uint8Array(romBuffer), romPointer);
            api.loadROM();
            api.stop();
            api.resetState();
            api.start();

            const oldOnMsg = PThread.runningWorkers[PThread.runningWorkers.length - 1].onmessage;
            PThread.runningWorkers[PThread.runningWorkers.length - 1].onmessage = msg => {
                if (msg.data.cmd == "updateCanvas") {
                    updateCanvas(msg.data.data);
                } else {
                    oldOnMsg(msg);
                }
            }
        };
    })
};

function updateCanvas(videoPointer) {
    const WIDTH = 64, HEIGHT = 32;
    video = new Uint32Array(Module.HEAPU32.buffer, videoPointer, WIDTH * HEIGHT);
    const canvas = document.getElementById("canvas");
    const ctx = canvas.getContext('2d');
    const imageData = ctx.createImageData(WIDTH, HEIGHT);

    for (let i = 0; i < WIDTH * HEIGHT; i++) {
        imageData.data[i * 4] = imageData.data[i * 4 + 1] = imageData.data[i * 4 + 2] = video[i] & 0xFF;
        imageData.data[i * 4 + 3] = 0xFF;
    }

    ctx.imageSmoothingEnabled = false;
    ctx.putImageData(imageData, 0, 0);
    ctx.drawImage(canvas, 0, 0, WIDTH, HEIGHT, 0, 0, canvas.clientWidth, canvas.clientHeight);
}

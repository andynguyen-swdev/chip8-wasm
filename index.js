let api;

Module.onRuntimeInitialized = async _ => {
    api = {
        getVideoPointer: Module.cwrap('getVideoPointer', 'number', []),
        allocateROM: Module.cwrap('allocateROM', 'number', ['number']),
        loadROM: Module.cwrap('loadROM', 'void', []),
        getROMPointer: Module.cwrap('getROMPointer', 'number', []),
        setDelay: Module.cwrap('setDelay', 'void', ['number']),
        start: Module.cwrap('start', 'void', []),
        stop: Module.cwrap('stop', 'void', []),
        resetState: Module.cwrap('resetState'),
        setKey: Module.cwrap('setKey'),
    };

    api.setDelay(1000 / 60);

    const romSelectElement = document.getElementById("rom_list");
    const romList = await fetch('roms/roms.json').then(res => res.json());
    for (const rom of romList) {
        const option = document.createElement("option");
        option.value = rom.file;
        option.innerText = rom.title;
        romSelectElement.appendChild(option);
    }
    romSelectElement.addEventListener("change", async ev => {
        if (ev.target.selectedIndex === 0) return;
        ev.target.blur();

        const rom = romList[ev.target.selectedIndex - 1];
        const romData = await fetch('roms/' + rom.file).then(res => res.arrayBuffer());

        const romPointer = api.allocateROM(romData.byteLength);
        // noinspection JSCheckFunctionSignatures
        Module.HEAPU8.set(new Uint8Array(romData), romPointer);
        api.stop();
        api.resetState();
        api.loadROM();
        api.start();
    })

    const keyMappings = {
        "1": 1,
        "2": 2,
        "3": 3,
        "4": 0xC,
        "q": 4,
        "w": 5,
        "e": 6,
        "r": 0xD,
        "a": 7,
        "s": 8,
        "d": 9,
        "f": 0xE,
        "z": 0xA,
        "x": 0,
        "c": 0xB,
        "v": 0xF,
    }
    document.addEventListener("keydown", ev => {
        const id = keyMappings[ev.key];
        if (id != null) {
            api.setKey(id, true);
        }
    });
    document.addEventListener("keyup", ev => {
        const id = keyMappings[ev.key];
        if (id != null) {
            api.setKey(id, false);
        }
    });
};

function updateCanvas(videoPointer) {
    const WIDTH = 64, HEIGHT = 32;
    const canvas = document.getElementById("canvas");
    const ctx = canvas.getContext('2d');
    const imageData = ctx.createImageData(WIDTH, HEIGHT);
    const video = new Uint32Array(Module.HEAPU32.buffer, videoPointer, WIDTH * HEIGHT);

    for (let i = 0; i < WIDTH * HEIGHT; i++) {
        const color = video[i] & 0xFF;
        imageData.data[i * 4] = imageData.data[i * 4 + 1] = imageData.data[i * 4 + 2] = color;
        imageData.data[i * 4 + 3] = 0xFF;
    }

    ctx.imageSmoothingEnabled = false;
    ctx.putImageData(imageData, 0, 0);
    ctx.drawImage(canvas, 0, 0, WIDTH, HEIGHT, 0, 0, canvas.clientWidth, canvas.clientHeight);
}

const chip8 = {
    defaultKeyMapping: {
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
    },

    initialize: async function () {
        this.api = {
            getVideoPointer: Module.cwrap('getVideoPointer', 'number', []),
            allocateROM: Module.cwrap('allocateROM', 'number', ['number']),
            loadROM: Module.cwrap('loadROM', 'void', []),
            getROMPointer: Module.cwrap('getROMPointer', 'number', []),
            resetState: Module.cwrap('resetState'),
            setKey: Module.cwrap('setKey'),
            cycle: Module.cwrap('cycle'),
        };

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
            const romPointer = this.api.allocateROM(romData.byteLength);
            Module.HEAPU8.set(new Uint8Array(romData), romPointer);

            this.keyMapping = rom.keyMapping || this.defaultKeyMapping;

            this.stop();
            this.api.resetState();
            this.api.loadROM();
            this.start();
        })

        document.addEventListener("keydown", ev => {
            const id = this.keyMapping && this.keyMapping[ev.key];
            if (id != null) {
                this.api.setKey(id, true);
            }
        });
        document.addEventListener("keyup", ev => {
            const id = this.keyMapping && this.keyMapping[ev.key];
            if (id != null) {
                this.api.setKey(id, false);
            }
        });
        document.addEventListener("keypress", ev => {
            if (ev.key == "Backspace") this.api.resetState();
        })
    },

    start: function () {
        this.intervalID = setInterval(this.api.cycle, 1000 / 60);
    },

    stop: function () {
        if (!this.intervalID) return
        clearInterval(this.intervalID);
    },

    updateCanvas: function (videoPointer) {
        const WIDTH = 64, HEIGHT = 32, SCALE = 10;
        const canvas = document.getElementById("canvas");
        const ctx = canvas.getContext('2d');

        const video = new Uint32Array(Module.HEAPU32.buffer, videoPointer, WIDTH * HEIGHT * 4);
        const imageData = ctx.getImageData(0, 0, WIDTH, HEIGHT);

        ctx.imageSmoothingEnabled = false;
        ctx.clearRect(0, 0, WIDTH * SCALE, HEIGHT * SCALE);

        for (let i = 0; i < video.length; i++) {
            imageData.data[i * 4] = imageData.data[i * 4 + 1] = imageData.data[i * 4 + 2] = video[i] & 0xFF;
            imageData.data[i * 4 + 3] = 0xFF;
        }

        ctx.putImageData(imageData, 0, 0);
        ctx.drawImage(canvas, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH * SCALE, HEIGHT * SCALE);

        ctx.strokeStyle = "#333";
        for (let i = 1; i < HEIGHT; i++) {
            ctx.beginPath();
            ctx.moveTo(0, i * SCALE);
            ctx.lineTo(WIDTH * SCALE, i * SCALE);
            ctx.stroke();
        }
        for (let i = 1; i < WIDTH; i++) {
            ctx.beginPath();
            ctx.moveTo(i * SCALE, 0);
            ctx.lineTo(i * SCALE, HEIGHT * SCALE);
            ctx.stroke();
        }
    }

}

Module.onRuntimeInitialized = async _ => {
    await chip8.initialize();
};

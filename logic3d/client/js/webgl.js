window.onload = gInit;
const HTML_CANVAS = "glCanvas";
const HTML_SAVE_BUTTON = "saveButton";
const HTML_NEW_WORLD_BUTTON = "newWorldButton";
const HTML_SAVE_FILE_BUTTON = "saveFileButton";
const HTML_LOAD_FILE_INPUT = "loadFileInput";
const HTML_SET_POS_BUTTON = "setPosButton";
const HTML_COPY_BUTTON = "copyButton";
const HTML_PASTE_BUTTON = "pasteButton";
const HTML_UNDO_BUTTON = "undoButton";
const HTML_FILL_BUTTON = "fillButton";
const HTML_FPS_LABEL = "fpsLabel";

const gVERTEX_ARRAY_STATIC = 0;
const gVERTEX_ARRAY_DYNAMIC = 1;

const gMODEL_NICE_RECT = 0;
const gMODEL_NICE_TRI = 1;
const gMODEL_NICE_CUBE = 2;
const gMODEL_CROSSHAIR = 3;

function gInit() {
	const near = 0.1;
	const far = 200;
	const widthRatio = 2; // 90 degrees
	renderInit(near, far, widthRatio);
	gKeysDown = {};
	gMouseMove = {
		leftRight: 0,
		upDown: 0
	};
	gInitVertexArrays();
	gInitModels();
	gInitObjects();
	logicInit();
	worldInit();
	worldeditInit();
	gPrevFrameTimestamp = performance.now();
	window.requestAnimationFrame(gMainLoop);
	window.onkeydown = gOnKeyDown;
	window.onkeyup = gOnKeyUp;

	gAvgFrameTime = 0;
	gFrameCount = 0;
	gFrameCountStartTime = performance.now();
	gFpsLabel = document.getElementById(HTML_FPS_LABEL);
	gFpsLabel.innerHTML = "FPS: ";

	gCurrentBlock = 0;
	gPointerLocked = false;
	renderCanvas.requestPointerLock = renderCanvas.requestPointerLock || renderCanvas.mozRequestPointerLock;
	document.exitPointerLock = document.exitPointerLock || document.mozExitPointerLock;
	document.addEventListener("pointerlockchange", gOnPointerLockChange, false);
	document.addEventListener("mozpointerlockchange", gOnPointerLockChange, false);
	renderCanvas.onmousedown = function(e) {
		if (!gPointerLocked) {
			renderCanvas.requestPointerLock();
		} else {
			if (e.button === 0) {
				let pos = worldGetInteractPos(false);
				if (pos !== null) {
					let oldBlock = worldGetBlock(pos.x, pos.y, pos.z);
					worldSetBlock(pos.x, pos.y, pos.z, 0);
					if (blockIsWire(oldBlock) || oldBlock === blockTYPE_INPUT) {
						logicCompileAll();
					} else if (blockIsLogic(oldBlock)) {
						let index = -1;
						for (let i = 0; i < logicNorObjects.length; ++i) {
							let logicObject = logicNorObjects[i];
							if (logicObject.x === pos.x && logicObject.y === pos.y && logicObject.z === pos.z) {
								index = i;
								break;
							}
						}
						if (index !== -1) logicNorObjects.splice(index, 1);
						index = -1;
						for (let i = 0; i < logicOrObjects.length; ++i) {
							let logicObject = logicOrObjects[i];
							if (logicObject.x === pos.x && logicObject.y === pos.y && logicObject.z === pos.z) {
								index = i;
								break;
							}
						}
						if (index !== -1) logicOrObjects.splice(index, 1);
						logicCompileAll();
					}
				}
			} else if (e.button === 1) {
				let pos = worldGetInteractPos(false);
				if (pos !== null) {
					gCurrentBlock = (worldGetBlock(pos.x, pos.y, pos.z));
				}
			} else if (e.button === 2) {
				if (gCurrentBlock !== 0) {
					let pos = worldGetInteractPos(true);
					if (pos !== null) {
						worldSetBlock(pos.x, pos.y, pos.z, gCurrentBlock);
						if (blockIsWire(gCurrentBlock) || gCurrentBlock === blockTYPE_INPUT) {
							logicCompileAll();
						}else if (gCurrentBlock === blockTYPE_NOR_OFF) {
							let logicObject = new LogicObject(pos.x, pos.y, pos.z, 0);
							logicNorObjects.push(logicObject);
							logicCompileAll();
						} else if (gCurrentBlock === blockTYPE_NOR_ON) {
							let logicObject = new LogicObject(pos.x, pos.y, pos.z, 1);
							logicNorObjects.push(logicObject);
							logicCompileAll();
						} else if (gCurrentBlock === blockTYPE_OR_OFF) {
							let logicObject = new LogicObject(pos.x, pos.y, pos.z, 0);
							logicOrObjects.push(logicObject);
							logicCompileAll();
						} else if (gCurrentBlock === blockTYPE_OR_ON) {
							let logicObject = new LogicObject(pos.x, pos.y, pos.z, 1);
							logicOrObjects.push(logicObject);
							logicCompileAll();
						}
					}
				}
			}
		}
	}
}
function gMainLoop(timestamp) {
	if (timestamp - gFrameCountStartTime >= 1000) {
		gFpsLabel.innerHTML = "FPS: " + Math.round(gFrameCount*1000/(timestamp - gFrameCountStartTime));
		//console.log("Frametime: " + gAvgFrameTime);
		gFrameCountStartTime = timestamp;
		gFrameCount = 0;
	}
	++gFrameCount;

	let deltaTime = timestamp - gPrevFrameTimestamp;
	gPrevFrameTimestamp = timestamp;
	let leftRight = 0, frontBack = 0, upDown = 0;
	let speed = 1/200;
	if (gKeysDown["w"]) frontBack += speed;
	if (gKeysDown["s"]) frontBack -= speed;
	if (gKeysDown["d"]) leftRight += speed;
	if (gKeysDown["a"]) leftRight -= speed;
	if (gKeysDown["shift"]) upDown -= speed;
	if (gKeysDown[" "]) upDown += speed;
	let sin = Math.sin(renderCamera.yAngle), cos = Math.cos(renderCamera.yAngle);
	let deltaZ = -frontBack*cos - leftRight*sin;
	let deltaX = leftRight*cos - frontBack*sin;
	renderCamera.z += deltaZ*deltaTime;
	renderCamera.x += deltaX*deltaTime;
	renderCamera.y += upDown*deltaTime;

	let sensitivity = 1 / 200;
	renderCamera.yAngle -= gMouseMove.leftRight*sensitivity;
	renderCamera.xAngle -= gMouseMove.upDown*sensitivity;
	if (renderCamera.xAngle < -Math.PI/2) renderCamera.xAngle = -Math.PI/2;
	if (renderCamera.xAngle > Math.PI/2) renderCamera.xAngle = Math.PI/2;
	gMouseMove.leftRight = 0;
	gMouseMove.upDown = 0;

	let cubeModel = gModels[gMODEL_NICE_CUBE];
	let len = cubeModel.instances.length;
	for (let i = 0; i < len; ++i) {
		cubeModel.instances[i].x += (Math.random() - 0.5)/8;
		cubeModel.instances[i].y += (Math.random() - 0.5)/8;
		cubeModel.instances[i].z += (Math.random() - 0.5)/8;
		cubeModel.instances[i].xAngle += (Math.random() - 0.5)/8;
		cubeModel.instances[i].yAngle += (Math.random() - 0.5)/8;
		cubeModel.instances[i].zAngle += (Math.random() - 0.5)/8;
	}
	logicUpdate();
	gDrawScene();
	window.requestAnimationFrame(gMainLoop);
	gAvgFrameTime += (performance.now() - timestamp - gAvgFrameTime)/60;
}
function gDrawScene() {
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	worldDraw();
	for (let i = 0; i < gVertexArrays.length; ++i) {
		gVertexArrays[i].drawModels();
	}
}
function gOnPointerLockChange() {
	if (gPointerLocked) {
		document.removeEventListener("mousemove", gOnMouseMove, false);
	} else {
		document.addEventListener("mousemove", gOnMouseMove, false);
	}
	gPointerLocked = !gPointerLocked;
}
function gOnMouseMove(e) {
	gMouseMove.leftRight += e.movementX;
	gMouseMove.upDown += e.movementY;
}
function gOnKeyDown(e) {
	let character = e.key.toLowerCase();
	gKeysDown[character] = true;
	switch (character) {
	case "0":
		gCurrentBlock = 0;
		break;
	case "1":
		gCurrentBlock = blockTYPE_NOR_ON;
		break;
	case "2":
		gCurrentBlock = blockTYPE_INPUT;
		break;
	case "3":
		gCurrentBlock = blockTYPE_OR_OFF;
		break;
	case "4":
		gCurrentBlock = blockTYPE_WIRE1;
		break;
	case "5":
		gCurrentBlock = blockTYPE_WIRE2;
		break;
	case "6":
		gCurrentBlock = blockTYPE_GRASS;
		break;
	case "7":
		gCurrentBlock = blockTYPE_DIRT;
		break;
	case " ":
		e.preventDefault();
		break;
	}
}
function gOnKeyUp(e) {
	gKeysDown[e.key.toLowerCase()] = false;
}
function gInitObjects() {
	let model = gModels[gMODEL_NICE_RECT];
	model.addInstance(new RenderInstance(0, 0, -20, 1, 0, 0, 0));
	model.addInstance(new RenderInstance(-20, 0, -40, 1, 0, 0, 0));
	model.finalizeInstances();

	model = gModels[gMODEL_NICE_TRI];
	model.addInstance(new RenderInstance(0, 0, -10, 1, 0, 0, 0));
	model.addInstance(new RenderInstance(10, 0, -15, 1, 0, 0, 0));
	model.finalizeInstances();

	model = gModels[gMODEL_NICE_CUBE];
	model.addInstance(new RenderInstance(2, -1, -5, 1, 0, 0, 0));
	model.addInstance(new RenderInstance(-2, 5, -10, 1, 0, 0, 0));
	for (let i = 0; i < 0; ++i) {
		let maxDist = 150;
		let xAngle = Math.random()*2*Math.PI, yAngle = Math.random()*2*Math.PI, zAngle = Math.random()*2*Math.PI;
		model.addInstance(new RenderInstance((Math.random() - 0.5)*maxDist, (Math.random() - 0.5)*maxDist, (Math.random() - 0.5)*maxDist, Math.random(), xAngle, yAngle, zAngle));
	}
	model.finalizeInstances();

	model = gModels[gMODEL_CROSSHAIR];
	model.addInstance(new RenderInstance(0, 0, -0.1, 0.001, 0, 0, 0));
	model.finalizeInstances();
}
function gInitModels() {
	gModels = [];
	let vertices = [
		-4, 3, 0, 1, 1, 1,
		-4, -3, 0, 1, 0, 0,
		4, 3, 0, 0, 1, 0,
		4, -3, 0, 0, 0, 1
	];
	let indices = [
		0, 1, 2, 3
	];
	gModels[gMODEL_NICE_RECT] = new RenderModel(vertices, indices, gl.TRIANGLE_STRIP, true);

	vertices = [
		0, 0.5, 0, 0, 1, 1,
		-0.5, -0.5, 1, 1, 0, 1,
		0.5, -0.5, 0, 1, 1, 0,
	];
	indices = [
		0, 1, 2
	];
	gModels[gMODEL_NICE_TRI] = new RenderModel(vertices, indices, gl.TRIANGLES, true);

	vertices = [
		// Front
		-1, 1, 1, 0, 1, 1,
		-1, -1, 1, 0, 1, 1,
		1, 1, 1, 0, 1, 1,
		1, -1, 1, 0, 1, 1,
		// Back
		-1, 1, -1, 1, 0, 1,
		-1, -1, -1, 1, 0, 1,
		1, 1, -1, 1, 0, 1,
		1, -1, -1, 1, 0, 1,
		// Top
		-1, 1, -1, 1, 1, 0,
		-1, 1, 1, 1, 1, 0,
		1, 1, -1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		// Bottom
		-1, -1, -1, 1, 0, 0,
		-1, -1, 1, 1, 0, 0,
		1, -1, -1, 1, 0, 0,
		1, -1, 1, 1, 0, 0,
		// Right
		1, 1, 1, 0, 1, 0,
		1, -1, 1, 0, 1, 0,
		1, 1, -1, 0, 1, 0,
		1, -1, -1, 0, 1, 0,
		// Left
		-1, 1, 1, 0, 0, 1,
		-1, -1, 1, 0, 0, 1,
		-1, 1, -1, 0, 0, 1,
		-1, -1, -1, 0, 0, 1,
	];
	indices = [
		0, 1, 2,	2, 1, 3,
		4, 6, 5, 	5, 6, 7,

		8, 9, 10,	10, 9, 11,
		12, 14, 13, 	13, 14, 15,

		16, 17, 18,	18, 17, 19,
		20, 22, 21, 	21, 22, 23,
	];
	gModels[gMODEL_NICE_CUBE] = new RenderModel(vertices, indices, gl.TRIANGLES, true);

	vertices = [
		-0.5, 0.5, 0, 0.0, 0.5, 0.5,
		-0.5, -0.5, 0, 0.0, 0.5, 0.5,
		0.5, 0.5, 0, 0.0, 0.5, 0.5,
		0.5, -0.5, 0, 0.0, 0.5, 0.5,
	];
	indices = [
		0, 1, 2, 3
	];
	gModels[gMODEL_CROSSHAIR] = new RenderModel(vertices, indices, gl.TRIANGLE_STRIP, false);

	gVertexArrays[gVERTEX_ARRAY_STATIC].addModel(gModels[gMODEL_NICE_CUBE]);
	gVertexArrays[gVERTEX_ARRAY_STATIC].addModel(gModels[gMODEL_NICE_TRI]);
	gVertexArrays[gVERTEX_ARRAY_STATIC].addModel(gModels[gMODEL_NICE_RECT]);
	gVertexArrays[gVERTEX_ARRAY_STATIC].addModel(gModels[gMODEL_CROSSHAIR]);
	gVertexArrays[gVERTEX_ARRAY_STATIC].finalizeModels();
}
function gInitVertexArrays() {
	gVertexArrays = [];
	gVertexArrays[gVERTEX_ARRAY_STATIC] = new RenderVertexArray(gl.STATIC_DRAW, gl.UNSIGNED_BYTE);
}
var gProjectionMatrix;
var gModels;
var gVertexArrays;
var gPrevFrameTimestamp;
var gKeysDown;
var gMouseMove;
var gPointerLocked;
var gCurrentBlock;
var gFrameCount;
var gFrameCountStartTime;
var gAvgFrameTime;
var gFpsLabel;
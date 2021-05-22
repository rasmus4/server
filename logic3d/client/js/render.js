const renderVertexShaderSource = `
	precision mediump float;
	attribute vec4 aVertexPosition;
	attribute vec4 aVertexColor;
	attribute mat4 aModelViewMatrix;

	uniform mat4 uProjectionMatrix;

	varying vec4 vColor;

	void main() {
		gl_Position = uProjectionMatrix * aModelViewMatrix * aVertexPosition;
		vColor = aVertexColor;
	}
`;
const renderFragmentShaderSource = `
	precision mediump float;
	varying vec4 vColor;

	void main() {
		gl_FragColor = vColor;
	}
`;
const renderPositionComponents = 3;
const renderPositionTypeSize = 4;
const renderColorComponents = 3;
const renderColorTypeSize = 4;
const renderVertexComponents = renderPositionComponents + renderColorComponents;
const renderVertexSize = renderPositionTypeSize*renderPositionComponents + renderColorTypeSize*renderColorComponents;
const renderMvpSize = 64;
var renderPositionType;
var renderColorType;

class RenderInstance {
	constructor(x, y, z, scale, xAngle, yAngle, zAngle) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.scale = scale;
		this.xAngle = xAngle;
		this.yAngle = yAngle;
		this.zAngle = zAngle;
	}
}

class RenderModel {
	constructor(vertices, indices, drawOperation, isRelativeCamera) {
		this.vertices = vertices;
		this.indices = indices;
		this.drawOperation = drawOperation;
		this.instances = [];
		this.startElementOffset = -1;
		this.mvps = null;
		this.isRelativeCamera = isRelativeCamera;
	}
	addInstance(instance) {
		this.instances.push(instance);
	}
	removeInstance(instance) {
		let index = this.instances.indexOf(instance);
		if (index !== -1) {
			this.instances.splice(index, 1);
		}
	}
	finalizeInstances() {
		this.mvps = mat4ArrayCreateZero(this.instances.length);
	}
	updateMvps() {
		let instancesLength = this.instances.length;
		if (this.isRelativeCamera) {
			for (let i = 0; i < instancesLength; ++i) {
				let instance = this.instances[i];
				mat4ArrayScaleTranslation(this.mvps, i, instance.scale, instance.x - renderCamera.x, instance.y - renderCamera.y, instance.z - renderCamera.z);
				mat4ArrayRotateRelX(this.mvps, i, instance.xAngle);
				mat4ArrayRotateRelY(this.mvps, i, instance.yAngle);
				mat4ArrayRotateRelZ(this.mvps, i, instance.zAngle);
				mat4ArrayRotateY(this.mvps, i, -renderCamera.yAngle);
				mat4ArrayRotateX(this.mvps, i, -renderCamera.xAngle);
			}
		} else {
			for (let i = 0; i < instancesLength; ++i) {
				let instance = this.instances[i];
				mat4ArrayScaleTranslation(this.mvps, i, instance.scale, instance.x, instance.y, instance.z);
				mat4ArrayRotateRelX(this.mvps, i, instance.xAngle);
				mat4ArrayRotateRelY(this.mvps, i, instance.yAngle);
				mat4ArrayRotateRelZ(this.mvps, i, instance.zAngle);
			}
		}
	}
}

class RenderVertexArray {
	constructor(glBufferUsage, glIndexType) {
		this.glBufferUsage = glBufferUsage;
		this.glIndexType = glIndexType;
		switch (glIndexType) {
			case gl.UNSIGNED_INT:
				this.indexTypeSize = 4;
				break;
			case gl.UNSIGNED_SHORT:
				this.indexTypeSize = 2;
				break;
			case gl.UNSIGNED_BYTE:
				this.indexTypeSize = 1;
				break;
			default:
				throw "Invalid glIndexType";
		}
		this.models = [];
		this.vertexBufferLength = -1;
		this.indexBufferLength = -1;
		this.mvpBufferLength = -1;
		this.vertexArray = gl.createVertexArray();
		this.vertexBuffer = gl.createBuffer();
		this.indexBuffer = gl.createBuffer();
		this.mvpBuffer = gl.createBuffer();

		gl.bindVertexArray(this.vertexArray);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
		gl.vertexAttribPointer(renderProgramInfo.aLocations.vertexPosition, renderPositionComponents, renderPositionType, false, renderVertexSize, 0);
		gl.enableVertexAttribArray(renderProgramInfo.aLocations.vertexPosition);
		gl.vertexAttribPointer(renderProgramInfo.aLocations.vertexColor, renderColorComponents, renderColorType, false, renderVertexSize, renderPositionTypeSize * renderPositionComponents);
		gl.enableVertexAttribArray(renderProgramInfo.aLocations.vertexColor);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.mvpBuffer);
		for (let i = 0; i < 4; ++i) {
			let loc = renderProgramInfo.aLocations.modelViewMatrix + i;
			gl.vertexAttribPointer(loc, 4, gl.FLOAT, false, renderMvpSize, i << 4);
			gl.vertexAttribDivisor(loc, 1);
			gl.enableVertexAttribArray(loc);
		}
	}
	addModel(model) {
		this.models.push(model);
	}
	removeModel(model) {
		let index = this.gameObjects.indexOf(model);
		if (index !== -1) {
			this.models.splice(index, 1);
		}
	}
	finalizeModels() {
		let verticesLength = 0, indicesLength = 0, modelsLength = this.models.length;
		for (let i = 0; i < modelsLength; ++i) {
			let model = this.models[i];
			verticesLength += model.vertices.length;
			indicesLength += model.indices.length;
		}

		let vertices = new Float32Array(verticesLength);

		let indices;
		if (this.glIndexType === gl.UNSIGNED_INT) {
			indices = new Uint32Array(indicesLength);
		} else if (this.glIndexType === gl.UNSIGNED_SHORT) {
			indices = new Uint16Array(indicesLength);
		} else if (this.glIndexType === gl.UNSIGNED_BYTE) {
			indices = new Uint8Array(indicesLength);
		}
		let verticesIndex = 0, indicesIndex = 0;
		for (let i = 0; i < modelsLength; ++i) {
			let model = this.models[i];
			vertices.set(model.vertices, verticesIndex);
			let vertexOffset = verticesIndex / renderVertexComponents;
			model.startElementOffset = this.indexTypeSize * indicesIndex;
			for (let j = 0; j < model.indices.length; ++j, ++indicesIndex) {
				indices[indicesIndex] = model.indices[j] + vertexOffset;
			}
			verticesIndex += model.vertices.length;
		}

		gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
		/*if (this.vertexBufferLength < vertices.length) {
			if (this.isDynamic) {
				this.vertexBufferLength = 2*vertices.length;
				gl.bufferData(gl.ARRAY_BUFFER, renderVertexSize*this.vertexBufferLength, gl.DYNAMIC_DRAW);
			} else {
				this.vertexBufferLength = vertices.length;
				gl.bufferData(gl.ARRAY_BUFFER, renderVertexSize*vertices.length, gl.STATIC_DRAW);
			}
		}*/
		//gl.bufferSubData(gl.ARRAY_BUFFER, 0, vertices);
		gl.bufferData(gl.ARRAY_BUFFER, vertices, this.glBufferUsage);

		gl.bindVertexArray(this.vertexArray);
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
		/*if (this.indexBufferLength < indices.length) {
			if (this.isDynamic) {
				this.indexBufferLength = 2*indices.length;
				gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, renderIndexSize*this.indexBufferLength, gl.DYNAMIC_DRAW);
			} else {
				this.indexBufferLength = indices.length;
				gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, renderIndexSize*indices.length, gl.DYNAMIC_DRAW);
			}
		}*/
		//gl.bufferSubData(gl.ELEMENT_ARRAY_BUFFER, 0, indices);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, this.glBufferUsage);
	}
	drawModels() {
		gl.bindVertexArray(this.vertexArray);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.mvpBuffer);
		let modelsLength = this.models.length;

		for (let i = 0; i < modelsLength; ++i) {
			let model = this.models[i];
			if (model.instances.length > 0 && model.indices.length > 0) {
				model.updateMvps();
				gl.bufferData(gl.ARRAY_BUFFER, model.mvps, gl.STREAM_DRAW);
				gl.drawElementsInstanced(model.drawOperation, model.indices.length, this.glIndexType, model.startElementOffset, model.instances.length);
			}
		}
	}
}
function renderInit(near, far, widthRatio) {
	renderCanvas = document.getElementById(HTML_CANVAS);
	if (!glTryInit(renderCanvas)) {
		alert("Couldn't initialize WebGL.");
		return;
	}
	renderColorType = gl.FLOAT;
	renderPositionType = gl.FLOAT;
	const program = glLoadShaderProgram(renderVertexShaderSource, renderFragmentShaderSource)
	renderProgramInfo = {
		program: program,
		aLocations: {
			vertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			vertexColor: gl.getAttribLocation(program, "aVertexColor"),
			modelViewMatrix: gl.getAttribLocation(program, "aModelViewMatrix")
		},
		uLocations: {
			projectionMatrix: gl.getUniformLocation(program, "uProjectionMatrix")
		}
	};
	renderCamera = {
		x: 32,
		y: 32,
		z: 32,
		xAngle: 0,
		yAngle: 0
	};

	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	gl.enable(gl.CULL_FACE);
	gl.cullFace(gl.BACK);
	//gl.enable(gl.BLEND);
	//gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	gl.useProgram(renderProgramInfo.program);

	const heightRatio = widthRatio*gl.canvas.clientHeight/gl.canvas.clientWidth;
	renderProjectionMatrix = mat4CreatePerspective(near*widthRatio, near*heightRatio, near, far);
	gl.uniformMatrix4fv(renderProgramInfo.uLocations.projectionMatrix, false, renderProjectionMatrix);
}
var renderCanvas;
var renderProgramInfo;
var renderCamera;
var renderProjectionMatrix;

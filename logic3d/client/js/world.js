const worldDefaultTestBlock = 1;
const worldDIRTY_STATIC_BIT = 0x1;
const worldDIRTY_DYNAMIC_BIT = 0x2;
class WorldChunk {
	constructor(xChunk, yChunk, zChunk) {
		this.xChunk = xChunk;
		this.yChunk = yChunk;
		this.zChunk = zChunk;
		this.blocks = new Uint8Array(1 << 12); //TODO: can be int8
		this.staticModel = new RenderModel([], [], gl.TRIANGLES, true);
		this.staticModel.addInstance(new RenderInstance(xChunk << 4, yChunk << 4, zChunk << 4, 1, 0, 0, 0));
		this.staticModel.finalizeInstances();
		this.staticVertexArray = new RenderVertexArray(gl.STATIC_DRAW, gl.UNSIGNED_SHORT);
		this.staticVertexArray.addModel(this.staticModel);
		this.dynamicModel = new RenderModel([], [], gl.TRIANGLES, true);
		this.dynamicModel.addInstance(new RenderInstance(xChunk << 4, yChunk << 4, zChunk << 4, 1, 0, 0, 0));
		this.dynamicModel.finalizeInstances();
		this.dynamicVertexArray = new RenderVertexArray(gl.DYNAMIC_DRAW, gl.UNSIGNED_SHORT);
		this.dynamicVertexArray.addModel(this.dynamicModel);
		this.dirty = worldDIRTY_STATIC_BIT | worldDIRTY_DYNAMIC_BIT;
	}
	updateModel(dynamic) {
		let vertexOffset = 0, verticesIndex = 0, indicesIndex = 0;
		let blocks = this.blocks;
		let rightChunk = worldGetChunk(this.xChunk + 1, this.yChunk, this.zChunk), leftChunk = worldGetChunk(this.xChunk - 1, this.yChunk, this.zChunk);
		let upChunk = worldGetChunk(this.xChunk, this.yChunk + 1, this.zChunk), downChunk = worldGetChunk(this.xChunk, this.yChunk - 1, this.zChunk);
		let frontChunk = worldGetChunk(this.xChunk, this.yChunk, this.zChunk + 1), backChunk = worldGetChunk(this.xChunk, this.yChunk, this.zChunk - 1);
		let dynamicCheck;
		if (dynamic) {
			dynamicCheck = 0;
			this.dirty &= ~worldDIRTY_DYNAMIC_BIT;
		} else {
			dynamicCheck = blockDYNAMIC_BIT;
			this.dirty &= ~worldDIRTY_STATIC_BIT;
		}
		let blockIndex = 0;
		for (let x = 0; x < 16; ++x) {
			let xPos = x + 1;
			for (let y = 0; y < 16; ++y) {
				let yPos = y + 1;
				for (let z = 0; z < 16; ++z, ++blockIndex) {
					let block = blocks[blockIndex];
					if (block === 0 || (block & blockDYNAMIC_BIT) === dynamicCheck)
						continue;
					let zPos = z + 1;
					let blockType = blockTypes[block];
					let otherR = blockType.otherR, otherG = blockType.otherG, otherB = blockType.otherB;
					let testBlock;
					if (z === 15) {
						if (frontChunk) {
							testBlock = frontChunk.blocks[blockIndex - 15];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex + 1];
					}
					if (testBlock === 0) {
						//z face
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR + 0.06;
						worldVertices[verticesIndex++] = otherG + 0.06;
						worldVertices[verticesIndex++] = otherB + 0.06;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
					if (z === 0) {
						if (backChunk) {
							testBlock = backChunk.blocks[blockIndex + 15];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex - 1];
					}
					if (testBlock === 0) {
						//-z face
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR + 0.06;
						worldVertices[verticesIndex++] = otherG + 0.06;
						worldVertices[verticesIndex++] = otherB + 0.06;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
					if (y === 15) {
						if (upChunk) {
							testBlock = upChunk.blocks[blockIndex - 240];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex + 16];
					}
					if (testBlock === 0) {
						//y face
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = blockType.upR + 0.06;
						worldVertices[verticesIndex++] = blockType.upG + 0.06;
						worldVertices[verticesIndex++] = blockType.upB + 0.06;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = blockType.upR;
						worldVertices[verticesIndex++] = blockType.upG;
						worldVertices[verticesIndex++] = blockType.upB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = blockType.upR;
						worldVertices[verticesIndex++] = blockType.upG;
						worldVertices[verticesIndex++] = blockType.upB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = blockType.upR;
						worldVertices[verticesIndex++] = blockType.upG;
						worldVertices[verticesIndex++] = blockType.upB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
					if (y === 0) {
						if (downChunk) {
							testBlock = downChunk.blocks[blockIndex + 240];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex - 16];
					}
					if (testBlock === 0) {
						//-y face
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR + 0.06;
						worldVertices[verticesIndex++] = otherG + 0.06;
						worldVertices[verticesIndex++] = otherB + 0.06;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
					if (x === 15) {
						if (rightChunk) {
							testBlock = rightChunk.blocks[blockIndex - 3840];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex + 256];
					}
					if (testBlock === 0) {
						//x face
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR + 0.06;
						worldVertices[verticesIndex++] = otherG + 0.06;
						worldVertices[verticesIndex++] = otherB + 0.06;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = xPos;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
					if (x === 0) {
						if (leftChunk) {
							testBlock = leftChunk.blocks[blockIndex + 3840];
						} else {
							testBlock = worldDefaultTestBlock;
						}
					} else {
						testBlock = blocks[blockIndex - 256];
					}
					if (testBlock === 0) {
						//-x face
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR + 0.06;
						worldVertices[verticesIndex++] = otherG + 0.06;
						worldVertices[verticesIndex++] = otherB + 0.06;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = zPos;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = yPos;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldVertices[verticesIndex++] = x;
						worldVertices[verticesIndex++] = y;
						worldVertices[verticesIndex++] = z;
						worldVertices[verticesIndex++] = otherR;
						worldVertices[verticesIndex++] = otherG;
						worldVertices[verticesIndex++] = otherB;
						worldIndices[indicesIndex++] = vertexOffset + 0;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 1;
						worldIndices[indicesIndex++] = vertexOffset + 2;
						worldIndices[indicesIndex++] = vertexOffset + 3;
						vertexOffset += 4;
					}
				}
			}
		}
		if (dynamic) {
			this.dynamicModel.vertices = worldVertices.subarray(0, verticesIndex);
			this.dynamicModel.indices = worldIndices.subarray(0, indicesIndex);
			this.dynamicVertexArray.finalizeModels();
		} else {
			this.staticModel.vertices = worldVertices.subarray(0, verticesIndex);
			this.staticModel.indices = worldIndices.subarray(0, indicesIndex);
			this.staticVertexArray.finalizeModels();
		}
	}
}
function worldGetChunk(xChunk, yChunk, zChunk) {
	if (xChunk < 0 || xChunk >= worldSizeXChunks || yChunk < 0 || yChunk >= worldSizeYChunks || zChunk < 0 || zChunk >= worldSizeZChunks) {
		return null;
	}
	return worldChunks[xChunk*worldSizeYZChunks + yChunk*worldSizeZChunks + zChunk];
}
function worldSetBlock(x, y, z, value) {
	let chunk = worldGetChunk(x >> 4, y >> 4, z >> 4);
	if (chunk === null) {
		return;
	}
	worldDirtyChunksAround(x, y, z);
	chunk.blocks[((x & 0xf) << 8) + ((y & 0xf) << 4) + (z & 0xf)] = value;
}
function worldUpdateBlock(x, y, z, value) {
	let chunk = worldGetChunk(x >> 4, y >> 4, z >> 4);
	if (chunk === null) {
		return;
	}
	chunk.dirty |= worldDIRTY_DYNAMIC_BIT;
	chunk.blocks[((x & 0xf) << 8) + ((y & 0xf) << 4) + (z & 0xf)] = value;
}
function worldGetBlock(x, y, z) {
	let chunk = worldGetChunk(x >> 4, y >> 4, z >> 4);
	if (chunk === null) {
		return 0;
	}
	return chunk.blocks[((x & 0xf) << 8) + ((y & 0xf) << 4) + (z & 0xf)];
}
function worldDraw() {
	let len = worldChunks.length;
	for (let i = 0; i < len; ++i) {
		let chunk = worldChunks[i];
		if (chunk) {
			if ((chunk.dirty & worldDIRTY_STATIC_BIT) !== 0) {
				chunk.updateModel(false);
			}
			chunk.staticVertexArray.drawModels();
			if ((chunk.dirty & worldDIRTY_DYNAMIC_BIT) !== 0) {
				chunk.updateModel(true);
			}
			chunk.dynamicVertexArray.drawModels();
		}
	}
}
function worldDirtyChunksAround(xBlock, yBlock, zBlock) {
	let xChunk = xBlock >> 4, yChunk = yBlock >> 4, zChunk = zBlock >> 4;
	let relX = xBlock & 0xf, relY = yBlock & 0xf, relZ = zBlock & 0xf;
	let selfChunk = worldGetChunk(xChunk, yChunk, zChunk);
	let dirtyBits = worldDIRTY_STATIC_BIT | worldDIRTY_DYNAMIC_BIT;
	selfChunk.dirty |= dirtyBits;
	if (relX === 0) {
		let chunk = worldGetChunk(xChunk - 1, yChunk, zChunk);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	} else if (relX === 15) {
		let chunk = worldGetChunk(xChunk + 1, yChunk, zChunk);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	}
	if (relY === 0) {
		let chunk = worldGetChunk(xChunk, yChunk - 1, zChunk);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	} else if (relY === 15) {
		let chunk = worldGetChunk(xChunk, yChunk + 1, zChunk);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	}
	if (relZ === 0) {
		let chunk = worldGetChunk(xChunk, yChunk, zChunk - 1);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	} else if (relZ === 15) {
		let chunk = worldGetChunk(xChunk, yChunk, zChunk + 1);
		if (chunk !== null) {
			chunk.dirty |= dirtyBits;
		}
	}
}
function worldGetInteractPos(infront) {
	let xAngle = renderCamera.xAngle;
	let yAngle = renderCamera.yAngle;
	let cosXAngle = Math.cos(xAngle);
	let componentZ = -cosXAngle*Math.cos(yAngle);
	let componentX = -cosXAngle*Math.sin(yAngle);
	let componentY = Math.sin(xAngle);

	let x = renderCamera.x;
	let y = renderCamera.y;
	let z = renderCamera.z;

	const maxDistanceSquared = 5*5;
	let xBlock, yBlock, zBlock;
	let prevXBlock, prevYBlock, prevZBlock;
	let nextX, nextY, nextZ;
	while (true) {
		let deltaX = x - renderCamera.x, deltaY = y - renderCamera.y, deltaZ = z - renderCamera.z;
		if (deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ > maxDistanceSquared) {
			return null;
		}
		prevXBlock = xBlock;
		prevYBlock = yBlock;
		prevZBlock = zBlock;
		xBlock = Math.floor(x), yBlock = Math.floor(y), zBlock = Math.floor(z);
		if (componentX < 0) {
			if (xBlock >= x) {
				--xBlock;
			}
			nextX = xBlock;
		} else {
			nextX = xBlock + 1;
		}
		if (componentY < 0) {
			if (yBlock >= y) {
				--yBlock;
			}
			nextY = yBlock;
		} else {
			nextY = yBlock + 1;
		}
		if (componentZ < 0) {
			if (zBlock >= z) {
				--zBlock;
			}
			nextZ = zBlock;
		} else {
			nextZ = zBlock + 1;
		}

		if (worldGetBlock(xBlock, yBlock, zBlock) !== 0) {
			let interactX, interactY, interactZ;
			if (infront) {
				if (prevXBlock === undefined) {
					return null;
				}
				return {x: prevXBlock, y: prevYBlock, z: prevZBlock};
			} else {
				return {x: xBlock, y: yBlock, z: zBlock};
			}
		}
		let timeX = (nextX - x)/componentX;
		let timeY = (nextY - y)/componentY;
		let timeZ = (nextZ - z)/componentZ;
		if (timeX < timeY) {
			if (timeX < timeZ) {
				x = nextX;
				y += timeX*componentY;
				z += timeX*componentZ;
			} else {
				z = nextZ;
				x += timeZ*componentX;
				y += timeZ*componentY;
			}
		} else {
			if (timeY < timeZ) {
				y = nextY;
				x += timeY*componentX;
				z += timeY*componentZ;
			} else {
				z = nextZ;
				x += timeZ*componentX;
				y += timeZ*componentY;
			}
		}
	}
}
function worldSave() {
	let worldString = "";
	for (let i = 0; i < worldChunks.length; ++i) {
		worldString += String.fromCharCode.apply(null, new Uint16Array(worldChunks[i].blocks.buffer));
	}
	window.localStorage.prevWorld = worldString;
}
function worldSaveFile() {
	let fileName = prompt("File name: ", "world");
	if (fileName === null) return;
	let data = [];
	for (let i = 0; i < worldChunks.length; ++i) {
		data.push(worldChunks[i].blocks);
	}
	let blob = new Blob(data, {type: "octet/stream"});
	let url = window.URL.createObjectURL(blob);
	worldDownloadLink.href = url;
	worldDownloadLink.download = fileName;
	worldDownloadLink.click();
	window.URL.revokeObjectURL(url);
}
function worldGenerate() {
	logicInit();
	for (let x = 0; x < worldSizeXChunks << 4; ++x) {
		for (let y = 0; y < worldSizeYChunks << 4; ++y) {
			for (let z = 0; z < worldSizeZChunks << 4; ++z) {
				//worldSetBlock(x, y, z, Math.random()*2);
				if (y < 15) {
					worldSetBlock(x, y, z, blockTYPE_DIRT);
				} else if (y > 15) {
					worldSetBlock(x, y, z, 0);
				} else {
					worldSetBlock(x, y, z, blockTYPE_GRASS);
				}
			}
		}
	}
}
function worldLoadFile(event) {
	let files = event.target.files;
	if (files.length > 0) {
		let file = files[0];
		let fileReader = new FileReader();
		fileReader.onload = function(event) {
			let buffer = event.target.result;
			let array = new Uint8Array(buffer);
			logicInit();
			let blockIndex = 0;
			for (let i = 0; i < worldChunks.length; ++i) {
				let chunk = worldChunks[i];
				for (let j = 0; j < 4096; ++j, ++blockIndex) {
					let block = array[blockIndex];
					chunk.blocks[j] = block;
					switch (block) {
						case blockTYPE_NOR_OFF:
							logicNorObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 0));
							break;
						case blockTYPE_NOR_ON:
							logicNorObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 1));
							break;
						case blockTYPE_OR_OFF:
							logicOrObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 0));
							break;
						case blockTYPE_OR_ON:
							logicOrObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 1));
							break;
					}
				}
				chunk.dirty = worldDIRTY_DYNAMIC_BIT | worldDIRTY_STATIC_BIT;
			}
			logicCompileAll();
		}
		fileReader.readAsArrayBuffer(file);
	}
	event.target.value = null;
}
function worldLoadPrev() {
	worldChunks = [];
	worldSizeXChunks = 10;
	worldSizeYChunks = 10;
	worldSizeZChunks = 10;
	worldSizeYZChunks = worldSizeYChunks*worldSizeZChunks;
	for (let xChunk = 0; xChunk < worldSizeXChunks; ++xChunk) {
		for (let yChunk = 0; yChunk < worldSizeYChunks; ++yChunk) {
			for (let zChunk = 0; zChunk < worldSizeZChunks; ++zChunk) {
				worldChunks[xChunk*worldSizeYZChunks + yChunk*worldSizeZChunks + zChunk] = new WorldChunk(xChunk, yChunk, zChunk);
			}
		}
	}
	let prevWorld = window.localStorage.prevWorld;
	if (prevWorld) {
		worldLoadFromString(prevWorld);
	} else {
		worldGenerate();
	}
}
function worldLoadFromString(world) {
	logicInit();
	let chunkBaseIndex = 0;
	for (let i = 0; i < worldChunks.length; ++i) {
		let chunk = worldChunks[i];
		for (let j = 0; j < 4096; ++j) {
			let block = world.charCodeAt(chunkBaseIndex + (j >>> 1));
			if (j & 0x1 === 1) {
				block = (block >>> 8) & 0xFF;
			} else {
				block &= 0xFF;
			}
			chunk.blocks[j] = block;
			switch (block) {
				case blockTYPE_NOR_OFF:
					logicNorObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 0));
					break;
				case blockTYPE_NOR_ON:
					logicNorObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 1));
					break;
				case blockTYPE_OR_OFF:
					logicOrObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 0));
					break;
				case blockTYPE_OR_ON:
					logicOrObjects.push(new LogicObject((chunk.xChunk << 4) + (j >>> 8), (chunk.yChunk << 4) + ((j >>> 4) & 0xf), (chunk.zChunk << 4) + (j & 0xf), 1));
					break;
			}
		}
		chunk.dirty = worldDIRTY_DYNAMIC_BIT | worldDIRTY_STATIC_BIT;
		chunkBaseIndex += 2048;
	}
	logicCompileAll();
}
function worldInit() {
	document.getElementById("loadFileInput").onchange = worldLoadFile;
	worldDownloadLink = document.createElement("a");
	worldDownloadLink.style = "display: none";
	document.body.appendChild(worldDownloadLink);
	document.getElementById(HTML_SAVE_BUTTON).onclick = function() {
		worldSave();
	};
	document.getElementById(HTML_NEW_WORLD_BUTTON).onclick = function() {
		worldGenerate();
	};
	document.getElementById(HTML_SAVE_FILE_BUTTON).onclick = function() {
		worldSaveFile();
	};

    worldLoadPrev();
	let maxVisibleQuads = (1 << 12)*3;
	let maxVertices = maxVisibleQuads*4;
	let maxIndices = maxVisibleQuads*6;
	worldVertices = new Float32Array(maxVertices*renderVertexComponents);
	worldIndices = new Float32Array(maxIndices);

	/*let block = 0;
	for (let x = 0; x < worldSizeXChunks*16; ++x) {
		for (let y = 0; y < worldSizeYChunks*16; ++y) {
			for (let z = 0; z < worldSizeZChunks*16; ++z) {
				//if (x === 0 || y === 0 || z === 0 || x === worldSizeXChunks*16 - 1 || y === worldSizeXChunks*16 - 1 || z === worldSizeXChunks*16 - 1) {
					//worldSetBlock(x, y, z, 0);
				//} else
				worldSetBlock(x, y, z, block);
				block = !block;
			}
			block = !block;
		}
		block = !block;
	}*/
	/*for (let x = 0; x < worldSizeXChunks*16; ++x) {
		for (let y = 0; y < worldSizeYChunks*16; ++y) {
			for (let z = 0; z < worldSizeZChunks*16; ++z) {
				if (y > 64) {
					worldSetBlock(x, y, z, 0);
				} else
				worldSetBlock(x, y, z, 1);
			}
		}
	}*/
}
var worldVertices;
var worldIndices;
var worldChunks;
var worldSizeXChunks;
var worldSizeYChunks;
var worldSizeZChunks;
var worldSizeYZChunks;
var worldDownloadLink;
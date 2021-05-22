function mat4CreateZero() {
	return new Float32Array(16);
}
function mat4CreateIdentity() {
	let mat = new Float32Array(16);
	mat[0] = 1;
	mat[5] = 1;
	mat[10] = 1;
	mat[15] = 1;
	return mat;
}
function mat4CreateTranslation(x, y, z) {
	let mat = mat4CreateIdentity();
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;
	return mat;
}
function mat4ArrayCreateZero(matCount) {
	return new Float32Array(matCount << 4);
}
function mat4ArrayCreateIdentities(matCount) {
	let mats = new Float32Array(matCount << 4);
	for (let i = 0; i < matCount; ++i) {
		let offset = i << 4;
		mats[offset] = 1;
		mats[offset + 5] = 1;
		mats[offset + 10] = 1;
		mats[offset + 15] = 1;
	}
	return mats;
}
function mat4ArrayScaleTranslation(matArray, matIndex, scale, x, y, z) {
	let offset = matIndex << 4;
	matArray[offset] = scale;
	matArray[offset + 1] = 0;
	matArray[offset + 2] = 0;
	matArray[offset + 4] = 0;
	matArray[offset + 5] = scale;
	matArray[offset + 6] = 0;
	matArray[offset + 8] = 0;
	matArray[offset + 9] = 0;
	matArray[offset + 10] = scale;
	matArray[offset + 15] = 1;
	matArray[offset + 12] = x;
	matArray[offset + 13] = y;
	matArray[offset + 14] = z;
}

function mat4ArrayRotateRelX(matArray, matIndex, angle) {
	//Assumes no translation
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v01 = matArray[offset + 4], v11 = matArray[offset + 5], v21 = matArray[offset + 6], v02 = matArray[offset + 8], v12 = matArray[offset + 9], v22 = matArray[offset + 10];
	matArray[offset + 4] = cos*v01 + sin*v02;
	matArray[offset + 5] = cos*v11 + sin*v12;
	matArray[offset + 6] = cos*v21 + sin*v22;
	matArray[offset + 8] = cos*v02 - sin*v01;
	matArray[offset + 9] = cos*v12 - sin*v11;
	matArray[offset + 10] = cos*v22 - sin*v21;
}
function mat4ArrayRotateRelY(matArray, matIndex, angle) {
	//Assumes no translation
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v00 = matArray[offset], v10 = matArray[offset + 1], v20 = matArray[offset + 2], v02 = matArray[offset + 8], v12 = matArray[offset + 9], v22 = matArray[offset + 10];
	matArray[offset] = cos*v00 - sin*v02;
	matArray[offset + 1] = cos*v10 - sin*v12;
	matArray[offset + 2] = cos*v20 - sin*v22;
	matArray[offset + 8] = cos*v02 + sin*v00;
	matArray[offset + 9] = cos*v12 + sin*v10;
	matArray[offset + 10] = cos*v22 + sin*v20;
}
function mat4ArrayRotateRelZ(matArray, matIndex, angle) {
	//Assumes no translation
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v00 = matArray[offset], v10 = matArray[offset + 1], v20 = matArray[offset + 2], v01 = matArray[offset + 4], v11 = matArray[offset + 5], v21 = matArray[offset + 6];
	matArray[offset] = cos*v00 + sin*v01;
	matArray[offset + 1] = cos*v10 + sin*v11;
	matArray[offset + 2] = cos*v20 + sin*v21;
	matArray[offset + 4] = cos*v01 - sin*v00;
	matArray[offset + 5] = cos*v11 - sin*v10;
	matArray[offset + 6] = cos*v21 - sin*v20;
}
function mat4ArrayRotateX(matArray, matIndex, angle) {
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v10 = matArray[offset + 1], v20 = matArray[offset + 2], v11 = matArray[offset + 5], v21 = matArray[offset + 6], v12 = matArray[offset + 9], v22 = matArray[offset + 10], v13 = matArray[offset + 13], v23 = matArray[offset + 14];
	matArray[offset + 1] = cos*v10 - sin*v20;
	matArray[offset + 2] = sin*v10 + cos*v20;
	matArray[offset + 5] = cos*v11 - sin*v21;
	matArray[offset + 6] = sin*v11 + cos*v21;
	matArray[offset + 9] = cos*v12 - sin*v22;
	matArray[offset + 10] = sin*v12 + cos*v22;
	matArray[offset + 13] = cos*v13 - sin*v23;
	matArray[offset + 14] = sin*v13 + cos*v23;
}
function mat4ArrayRotateY(matArray, matIndex, angle) {
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v00 = matArray[offset], v20 = matArray[offset + 2], v01 = matArray[offset + 4], v21 = matArray[offset + 6], v02 = matArray[offset + 8], v22 = matArray[offset + 10], v03 = matArray[offset + 12], v23 = matArray[offset + 14];
	matArray[offset] = cos*v00 + sin*v20;
	matArray[offset + 2] = -sin*v00 + cos*v20;
	matArray[offset + 4] = cos*v01 + sin*v21;
	matArray[offset + 6] = -sin*v01 + cos*v21;
	matArray[offset + 8] = cos*v02 + sin*v22;
	matArray[offset + 10] = -sin*v02 + cos*v22;
	matArray[offset + 12] = cos*v03 + sin*v23;
	matArray[offset + 14] = -sin*v03 + cos*v23;
}
function mat4ArrayRotateZ(matArray, matIndex, angle) {
	let offset = matIndex << 4;
	let cos = Math.cos(angle);
	let sin = Math.sin(angle);
	let v00 = matArray[offset], v10 = matArray[offset + 1], v01 = matArray[offset + 4], v11 = matArray[offset + 5], v02 = matArray[offset + 8], v12 = matArray[offset + 9], v03 = matArray[offset + 12], v13 = matArray[offset + 13];
	matArray[offset] = cos*v00 - sin*v10;
	matArray[offset + 1] = sin*v00 + cos*v10;
	matArray[offset + 4] = cos*v01 - sin*v11;
	matArray[offset + 5] = sin*v01 + cos*v11;
	matArray[offset + 8] = cos*v02 - sin*v12;
	matArray[offset + 9] = sin*v02 + cos*v12;
	matArray[offset + 12] = cos*v03 - sin*v13;
	matArray[offset + 13] = sin*v03 + cos*v13;
}
function mat4CreatePerspective(width, height, near, far) {
	let mat = new Float32Array(16);
	mat4Perspective(mat, width, height, near, far);
	return mat;
}
function mat4Perspective(mat, width, height, near, far) {
	mat[0] = 2*near/width;
	mat[5] = 2*near/height;
	mat[10] = -(near + far)/(far - near);
	mat[11] = -1;
	mat[14] = -(2*near*far/(far - near));
}
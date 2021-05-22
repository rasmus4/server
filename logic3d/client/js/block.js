const blockDYNAMIC_BIT = 0x80;
// Static
const blockTYPE_NONE = 0;
const blockTYPE_GRASS = 1;
const blockTYPE_DIRT = 2;
const blockTYPE_WIRE1 = 3;
const blockTYPE_WIRE2 = 4;
const blockTYPE_INPUT = 5;
// Dynamic
const blockTYPE_NOR_OFF = blockDYNAMIC_BIT | 0;
const blockTYPE_NOR_ON = blockDYNAMIC_BIT | 1;
const blockTYPE_OR_OFF = blockDYNAMIC_BIT | 2;
const blockTYPE_OR_ON = blockDYNAMIC_BIT | 3;

function blockIsLogic(block) {
	return (block & blockDYNAMIC_BIT) === blockDYNAMIC_BIT;
}
function blockIsWire(block) {
	return block === blockTYPE_WIRE1 || block === blockTYPE_WIRE2;
}

var blockTypes = [];
blockTypes[blockTYPE_GRASS] = {
	upR: 0.3,
	upG: 0.73,
	upB: 0.09,
	otherR: 0.35,
	otherG: 0.23,
	otherB: 0.05
};
blockTypes[blockTYPE_DIRT] = {
	upR: 0.35,
	upG: 0.23,
	upB: 0.05,
	otherR: 0.35,
	otherG: 0.23,
	otherB: 0.05
};
blockTypes[blockTYPE_WIRE1] = {
	upR: 0.5,
	upG: 0.5,
	upB: 0.5,
	otherR: 0.5,
	otherG: 0.5,
	otherB: 0.5
};
blockTypes[blockTYPE_WIRE2] = {
	upR: 0.2,
	upG: 0.2,
	upB: 0.5,
	otherR: 0.2,
	otherG: 0.2,
	otherB: 0.5
};
blockTypes[blockTYPE_INPUT] = {
	upR: 0.9,
	upG: 0.9,
	upB: 0.9,
	otherR: 0.9,
	otherG: 0.9,
	otherB: 0.9
};
blockTypes[blockTYPE_NOR_ON] = {
	upR: 1,
	upG: 0.2,
	upB: 0.2,
	otherR: 1,
	otherG: 0.2,
	otherB: 0.2
};
blockTypes[blockTYPE_NOR_OFF] = {
	upR: 0.25,
	upG: 0.2,
	upB: 0.2,
	otherR: 0.25,
	otherG: 0.2,
	otherB: 0.2
};
blockTypes[blockTYPE_OR_ON] = {
	upR: 0.2,
	upG: 1,
	upB: 0.2,
	otherR: 0.2,
	otherG: 1,
	otherB: 0.2
};
blockTypes[blockTYPE_OR_OFF] = {
	upR: 0.2,
	upG: 0.25,
	upB: 0.2,
	otherR: 0.2,
	otherG: 0.25,
	otherB: 0.2
};
function blockMat = ImageBlock2Col(image, blkSize)
	% input: h*w image, h,w % blkSize == 0,0
	% output: divide image into blocks, one block per column
	[w,h] = size(image);
	assert(mod(w, blkSize) == 0);
	assert(mod(h, blkSize) == 0);
	nBlkX = fix(w/blkSize);
	nBlkY = fix(h/blkSize);
	image = reshape(image, [blkSize nBlkY w]);
	image = permute(image, [1 3 2]);
	blockMat = reshape(image, [(blkSize*blkSize) (nBlkX*nBlkY)]);

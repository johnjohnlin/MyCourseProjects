function ret = SSIM(image1, image2, c1, c2)
	% input: 2 same size gray image
	assert(isequal(size(image1), size(image2)));
	image1 = ImageBlock2Col(image1, 8);
	image2 = ImageBlock2Col(image2, 8);
	uX = mean(image1);
	uY = mean(image2);
	vX = var(image1, 1);
	vY = var(image2, 1);
	uXuY = uX.*uY;
	covXY = mean(image1.*image2) - uXuY;
	ret = mean(((2*uXuY+c1).*(2*covXY+c2)) ./ ((uX.*uX+uY.*uY+c1).*(vX+vY+c2)));

function ret = cal_hu(img)
	u00 = raw_moments(img, 0, 0);
	e11 = central_moments(img, 1, 1, u00);
	e20 = central_moments(img, 2, 0, u00);
	e02 = central_moments(img, 0, 2, u00);
	e21 = central_moments(img, 2, 1, u00);
	e12 = central_moments(img, 1, 2, u00);
	e30 = central_moments(img, 3, 0, u00);
	e03 = central_moments(img, 0, 3, u00);
	I1 = e20 + e02;
	I2 = (e20-e02)^2 + 4*e11^2;
	I3 = (e30 - 3*e12)^2 + (3*e21 - e03)^2;
	I4 = (e30+e12)^2 + (e21+e03)^2;
	I5 =   (e30 - 3*e12) * (e30+e12) * ((e30+e12)^2 - 3*(e21+e03)^2)...
	     + (3*e21 - e03) * (e21+e03) * (3*(e30+e12)^2 - (e21+e03)^2);
	I6 = (e20-e02) * ((e30+e12)^2-(e21+e03^2)) + 4*e11*(e30+e12)*(e21+e03);
	I7 =   (3*e21-e03)*(e30+e12)*((e30+e12)^2 - 3*(e21+e03)^2)...
	     + (e30-3*e12)*(e21+e03)*(3*(e30+e12)^2 - (e21+e03)^2);
	ret = [I1 I2 I3 I4 I5 I6 I7]';
end

function outmom = raw_moments(im,i,j)
	outmom = sum(sum( ((1:size(im,1))'.^j * (1:size(im,2)).^i) .* im ));
end

function cmom = central_moments(im,i,j,rawm00)
	% normalized version
	irawm00 = 1.0 / rawm00;
	centroids = [raw_moments(im,1,0)*irawm00 , raw_moments(im,0,1)*irawm00];
	cmom = sum(sum( (([1:size(im,1)]-centroids(2))'.^j * ...
				 ([1:size(im,2)]-centroids(1)).^i) .* im ));
	cmom  = cmom * irawm00^(1+(i+j)/2);
end


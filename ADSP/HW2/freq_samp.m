function [fsFilter, x, sampleH] = freq_samp(k, H)
	% H: function, the normalized frequency response
	% k: tap of filter = 2k+1
	x = (0:(2*k)) * (1/(1+2*k));
	sampleH = H(x);
	fsFilter = circshift(ifft(sampleH), [0 k]);

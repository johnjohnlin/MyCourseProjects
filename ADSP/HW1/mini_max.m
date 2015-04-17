function [filt fr] = mini_max(initGuess, weight, reso, tb, k, hp)
	% input
	%     ini~ss: initial extrema col vector, length = k+2, no value in tb
	%     weight: weight of LF and HF, weight(1) = LF, weight(2) = HF
	%       reso: resolution of frequency response in passband & stopband (approximated)
	%         tb: transition band (2 elements, 0 <= tb(1) <= tb(2) <= 0.5)
	%          k: # taps of filter
	%         hp: true if high pass
	% return
	%     filt: 2k+1 symmetric filter coeff
	%       fr: frequency response
	width = 0.5 - tb(2) + tb(1);
	step = width / reso;
	x_lf = [0:step:tb(1)].';
	n_lf = length(x_lf);
	x_hf = [tb(2):step:0.5].';
	n_hf = length(x_hf);
	res_lf = double(~hp);
	res_hf = double( hp);
	xs = [x_lf; x_hf];
	tbc = mean(tb);

	% constants
	ONE_TO_K = double(1:k);
	K_ONES = double(ones(k+2, 1));  % actually, k+2
	MAX_ITER = 10;
	function [freqsWeight target_value] = update_iter(freqs)
		hfBool = freqs < tbc;
		freqsWeight = K_ONES;
		freqsWeight(2:2:end) = -1;
		freqsWeight(~hfBool) = freqsWeight(~hfBool) * (1/weight(1));
		freqsWeight( hfBool) = freqsWeight( hfBool) * (1/weight(2));
		target_value = K_ONES;
		target_value(~xor(hfBool, hp)) = 0;
	end

	% loop variables
	converged = false;
	freqs = initGuess;
	i = 0;
	while(~converged)
		[freqsWeight,target_value] = update_iter(freqs);
		A = [K_ONES cos(2*pi*freqs*ONE_TO_K) freqsWeight];
		s_and_e = A \ target_value
		curFR = ones(length(xs), 1) * s_and_e(1);
		for n=1:k
			curFR = curFR + s_and_e(n+1)*cos(2*pi*n*xs);
		end
		if (i == MAX_ITER)
			converged = true;
		end
		i = i + 1;
		plot(xs, curFR);

		% update frequency
		curFR_lf = curFR(1:n_lf);
		curFR_hf = curFR(n_lf+1:end);
		[minPeakVal,minLoc] = findpeaks(-curFR_lf);
		[maxPeakVal,maxLoc] = findpeaks( curFR_lf);
		maxErr_lf = max(abs([-minPeakVal; maxPeakVal]-res_lf));
		freqs_lf = [x_lf(minLoc); x_lf(maxLoc)];
		[minPeakVal,minLoc] = findpeaks(-curFR_hf);
		[maxPeakVal,maxLoc] = findpeaks( curFR_hf);
		maxErr_hf = max(abs([-minPeakVal; maxPeakVal]-res_hf));
		maxErr = max(maxErr_lf, maxErr_hf)
		freqs_hf = [x_hf(minLoc); x_hf(maxLoc)];
		freqs = [freqs_lf; freqs_hf];
		numFreqs = length(freqs);
		assert(numFreqs <= k+2);
		if (numFreqs < k+2)
			errsAtBoundary = abs([curFR_lf(1)-res_lf, curFR_lf(end)-res_lf, curFR_hf(1)-res_hf, curFR_hf(end)-res_hf]);
			[~,errOrder] = sort(errsAtBoundary);
			freqsAtBoundary = [x_lf(1) x_lf(end) x_hf(1) x_hf(end)];
			freqsAtBoundary = freqsAtBoundary(errOrder).';
			freqs = [freqs;freqsAtBoundary(1:(k+2-numFreqs))];
		end
		% sort: why necessary?
		freqs = sort(freqs);
		pause
	end
end

% ECE315H Beam-Forming from N amount of audio inputs
% Andrew DaCruz, Nathan Williams, Jacob Zylinski
% Task 1: Process beamforming using audio recordings on MATLAB
% This matlab code processes an audio file of a real signal and performs a
% fft to provide frequency, magnitude, and phase
% 10/8/2025

clear; clf; clc;

% Import the audio 

[y, Fs] = audioread('constant_5khz.m4a');

% extract mono channel audio (instead of stero audio)

y_mono = y(:,1);

% Definitions 


N = length(y_mono);     % number of samples
L = N/Fs;               % Length of signal (duration in s)
T = 1/Fs;               % Sampling period       
t = linspace(0,L,N);    % Time array

% Plot time-domain signal

plot(1000*t,y_mono)         
title("Signal in Time-Domain")
xlabel("t (milliseconds)")
ylabel("y(t)")

% Set up Fast Fourier Transform

Y = fft(y_mono);                % Y(f): signal in frequency domain
mag = abs(Y);                   % magnitude |Y(f)|
phase = angle(Y) * 180/pi;      % phase of Y(f), in degrees

% size of Y is same as input, so each frequency bin (index) is Fs/N (hz), 
% and indices 0:N/2 are pos frequency, N/2+1:N-1 are neg frequency

half_N = floor(N/2);            % half the samples to divide frequencies
f = (0:half_N) * (Fs/N);        % define positive frequency array
mag = mag(1:half_N+1);          % pos freq magnitude
phase = phase(1:half_N+1);      % pos freq phase

% Plot magnitude and phase of signal in frequency - domain

figure;
plot(f, mag, 'LineWidth', 1.5);
title("Magnitude of Signal in Frequency Domain");
xlabel("Frequency (Hz)");
ylabel("|Y(f)|");

figure;
plot(f, unwrap(phase), 'LineWidth', 1.5);
title("Phase of Signal in Frequency Domain");
xlabel("Frequency (Hz)");
ylabel("<Y(f) (deg)");


% set up data to use for beam forming

freq_mag_phase = [f(:), mag(:), phase(:)];
% Jacob Zylinski, Andrew DaCruz, Nathan Williams
% ECE 315H
% 8 October 2025
% Beamforming a simulated signal to find Angle of Arrival

clc;


% Load .txt file into matlab

file = 'test.txt'; 
fid = fopen(file, 'r');
text = fread(fid, '*char')';
fclose(fid);

% Find Sampling Rate in .txt file

rateExpr = 'Sampling Rate:\s*([\d.]+)';
rateMatch = regexp(text, rateExpr, 'tokens');
if ~isempty(rateMatch)
    fs = str2double(rateMatch{1}{1});
else
    error('Sampling rate not found.');
end
fprintf('Sampling Rate: %.2f Hz\n', fs);

% Find Mic Data

micSections = regexp(text, ...
    'Microphone\s+(\d+)[\s\S]*?(?=(Microphone\s+\d+|Sample\s+\d+|$))', ...
    'match');

micData = struct();

for i = 1:length(micSections)
    micID = str2double(regexp(micSections{i}, ...
        'Microphone\s+(\d+)', 'tokens', 'once'));
    
    % Voltages
    voltMatch = regexp(micSections{i}, ...
        'Voltages:\s*([\s\S]*?)(?:\n[A-Z]|$)', 'tokens');
    if ~isempty(voltMatch)
        volts = str2num(voltMatch{1}{1}); 
    else
        volts = [];
    end

    timeMatch = regexp(micSections{i}, ...
        'Timestamps:\s*([\s\S]*?)(?:\n[A-Z]|$)', 'tokens');
    if ~isempty(timeMatch)
        t = str2num(timeMatch{1}{1});
    else
        t = (0:length(volts)-1)/fs;
    end

    micData(micID).t = t(:);
    micData(micID).v = volts(:);
end

x = micData(2).v;
y = micData(1).v;
Fs = fs;

Fpass = 300;    % Hz

x = highpass(x,Fpass,Fs);
y = highpass(y,Fpass,Fs);

% extract mono channel audio (instead of stero audio)

y_mono = y;

% Definitions 

N = length(y_mono);     % number of samples
L = N/Fs;               % Length of signal (duration in s)
T = 1/Fs;               % Sampling period       
t = linspace(0,L,N);    % Time array

% Plot time-domain signal
figure;
plot(1000*t,y_mono, 1000*t, x)         
title("Signal in Time-Domain")
xlabel("t (milliseconds)")
ylabel("signal")
legend("Mic 1","Mic 2");


% Set up Fast Fourier Transform
X = fft(x);
magx = abs(X);
phasex = angle(X);

Y = fft(y_mono);                % Y(f): signal in frequency domain
mag = abs(Y);                   % magnitude |Y(f)|
phase = angle(Y);               % phase of Y(f), in radians

% size of Y is same as input, so each frequency bin (index) is Fs/N (hz), 
% and indices 0:N/2 are pos frequency, N/2+1:N-1 are neg frequency

half_N = floor(N/2);            % half the samples to divide frequencies
f = (0:half_N) * (Fs/N);        % define positive frequency array
mag = mag(1:half_N+1);          % pos freq magnitude
mag(1:3)=0;
phase = phase(1:half_N+1);      % pos freq phase

magx = magx(1:half_N+1);          % pos freq magnitude
magx(1:3)=0;
phasex = phasex(1:half_N+1);      % pos freq phase

% set up data to use for beam forming

freq_mag_phase_y = [f(:), mag(:), phase(:)];
freq_mag_phase_x = [f(:), magx(:), phasex(:)];

% Find transmitting frequency

[Mmaxy,indexy] = max(freq_mag_phase_y(:,2));
f0y = freq_mag_phase_y(indexy,1)

[Mmaxx,indexx] = max(freq_mag_phase_x(:,2));
f0x = freq_mag_phase_x(indexx,1)

check_f0 = f0y-f0x

f0 = (f0x + f0y)/2


% f0 = input("Please enter transmitting frequency in hz: ");

% Setup delay

delay = 0.75 * 1/Fs;   % seconds
for i = 1:length(freq_mag_phase_x)
    freq = freq_mag_phase_x(i,1);
    phasex(i,1) = phasex(i,1) - (2*pi*freq*delay);
end

freq_mag_phase_x = [f(:), magx(:), phasex(:)];


% Plot magnitude and phase of signal in frequency - domain

figure;
plot(f, mag, f, magx, 'LineWidth', 1.5);
title("Magnitude of Signal in Frequency Domain");
xlabel("Frequency (Hz)");
ylabel("mag");
legend("Mic 1","Mic 2");

figure;
plot(f, unwrap(phase), f, unwrap(phasex), 'LineWidth', 1.5);
title("Phase of Signal in Frequency Domain");
xlabel("Frequency (Hz)");
ylabel("< (f) (deg)");
legend("Mic 1","Mic 2");



% Set up beamforming

d = 5e-2;       % distance between microphones (m)
c = 343;        % speed of sound (m/s)

[~,indexx] = min(abs(freq_mag_phase_x(:,1) - f0));
[~,indexy] = min(abs(freq_mag_phase_y(:,1) - f0));

phi_x = freq_mag_phase_x(indexx,3) 
phi_y = freq_mag_phase_y(indexy,3)

dphi = angle(exp(1j*(phi_x - phi_y))) % delta phi, in rad, wrap [-pi,pi]

lambda = c/f0                           % wavelength, in m
ah = lambda / (2 * pi * d) * dphi


theta = asin(ah) * 180/pi

function [g,fs,block_len,num_chans,filt_lens,a,foff] = ...
                                                read_filterbank_bin(name)
                                            
% Determine sampling rate from filename
marker_pos = strfind(name,'_');
fs = str2double(name(marker_pos(1)+1:marker_pos(2)-1));
                                       
% Check file suffix, add if missing
if ~strcmp(name(end-3:end),'.lfb')
    name = [name,'.lfb'];
end
                                   
% Open file
fid = fopen(name,'rb');

% Read block length
block_len = fread(fid,[1 1],'uint16');

% Read number of channels
num_chans = fread(fid,[1 1],'uint16');

% Read frequency response supports
filt_lens = fread(fid,[num_chans 1],'uint16');

% Read downsampling factors
a = zeros(num_chans,2);

a(:,1) = fread(fid,[1 1],'uint16');
a(:,2) = fread(fid,[num_chans 1],'uint16');

% Read frequency offsets
foff = fread(fid,[num_chans 1],'uint16');

% Read cell array of filter frequency responses
g = cell(num_chans,1);
for kk = 1:num_chans
    g{kk} = fread(fid,[filt_lens(kk) 1],'float32');
end

% Close file stream
fclose(fid);                                          
                                            
function [g,fs,block_len,num_chans,a,fc,foff,filt_lens] = ...
                                                read_filterbank_bin(name,which)
                                            
% Determine sampling rate from filename
marker_pos = strfind(name,'_');
fs = str2double(name(1:marker_pos(1)-1));
                                       
% Check file suffix, add if missing
if ~strcmp(name(end-3:end),'.lfb')
    name = [name,'.lfb'];
end
                                   
% Open file
fid = fopen(name,'rb');

fseek(fid,0,1);
eof = ftell(fid);
fseek(fid,0,-1);

kk = 1;
cur_pos = 0;
while (kk < which && cur_pos < eof)
    lofbnext = fread(fid,[1 1],'uint32');
    new_pos = cur_pos+lofbnext;
    if new_pos < eof
        kk = kk+1;
        fseek(fid,lofbnext-4,0);
        cur_pos = new_pos;
    else
        error(['Filter bank #',num2str(which),' cannot be accessed. '...
            'File only contains ',num2str(kk),' filter banks.']);
    end
end

% Read filterbank length
lofb = fread(fid,[1 1],'uint32');

% Read block length
block_len = fread(fid,[1 1],'uint16');

% Read number of channels
num_chans = fread(fid,[1 1],'uint16');

% Read downsampling factors
a = zeros(num_chans,2);

a(:,1) = fread(fid,[1 1],'uint16');
a(:,2) = fread(fid,[num_chans 1],'uint16');

% Read center frequencies
fc = fread(fid,[num_chans 1],'float32');

% Read frequency offsets
foff = fread(fid,[num_chans 1],'uint16');

% Read frequency response supports
filt_lens = fread(fid,[num_chans 1],'uint16');

% Read cell array of filter frequency responses
g = cell(num_chans,1);
for kk = 1:num_chans
    g{kk} = fread(fid,[filt_lens(kk) 1],'float32');
end

% Close file stream
fclose(fid);                                      
                                            
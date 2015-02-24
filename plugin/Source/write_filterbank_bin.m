function write_filterbank_bin(name,g,a,block_len)

[row,col] = size(g);
if col > 1 
    if row > 1
        error('Filter banks for multichannel signals are not supported');
    end
    g = g.';
end

if block_len ~= filterbanklength(block_len,a)
    error('Filters cannot be instatnialized with block_len=%i.',block_len);
end

% Sanitize filters
[g,a] = filterbankwin(g,a,block_len);

% Convert a to the fractional-subsampling format
if all(a(:,2)==1)
    a(:,2) = block_len/a(:,1);
    a(:,1) = block_len;
end

% Precompute filters explicitly
g = comp_filterbank_pre(g,a,block_len);

% Save full file name
full_name = [num2str(block_len),'_',num2str(g{1}.fs),'_',name,'.lfb'];

% Determine number of channels
num_chans = numel(g);

% Determine frequency response supports from g
filt_lens = cellfun(@(gEl) numel(gEl.H),g);

% Prepare downsampling factors
afull = [a(1,1);a(:,2)];

% Determine frequency offset from g
foff = cellfun(@(gEl) gEl.foff,g);
foff = mod(foff,block_len);

% Extract frequency responses from g
g_mat = cellfun(@(gEl) gEl.H,g,'UniformOutput',false);
g_mat = cell2mat(g_mat);

% Write data in file, overwrite existing file
fid = fopen(full_name,'wb');
fwrite(fid,[block_len;num_chans;filt_lens;afull;foff],'uint16');
fwrite(fid,g_mat,'float32');

% Close file stream
fclose(fid);
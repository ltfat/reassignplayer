function write_filterbank_bin(name,g,a,block_len,fc,append)

fcGiven = 1;
if nargin < 6
    if nargin < 5
        fcGiven = 0;
    end    
    append = 1;
end

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

[hg, dg, ~] = comp_phasegradfilters(g,a,block_len);

% Save full file name
fs = g{1}.fs;
full_name = [num2str(fs),'_',name,'.lfb'];

% Determine number of channels
num_chans = numel(g);

% Determine frequency response supports from g
filt_lens = cellfun(@(gEl) numel(gEl.H),g);

% Prepare downsampling factors
afull = [a(1,1);a(:,2)];

% Determine normalized center frequencies
if fcGiven
  fc = cent_freqs(fs,fc);  
else
  fc = cent_freqs(g,block_len);
end

% Determine frequency offset from g
foff = cellfun(@(gEl) gEl.foff,g);
foff = mod(foff,block_len);

% Extract frequency responses from g
g_mat = cellfun(@(gEl) gEl.H,g,'UniformOutput',false);
g_mat = cell2mat(g_mat);

% Extract frequency responses from hg
hg_mat = cellfun(@(hgEl) hgEl.H,hg,'UniformOutput',false);
hg_mat = cell2mat(hg_mat);

% Extract frequency responses from dg
dg_mat = cellfun(@(dgEl) dgEl.H,dg,'UniformOutput',false);
dg_mat = cell2mat(dg_mat);

% Write filterbank data in file, append/overwrite existing file
if append
    fid = fopen(full_name,'ab');
    fidfgrad = fopen([full_name(1:end-4),'_fgrad.lfb'],'ab');
    fidtgrad = fopen([full_name(1:end-4),'_tgrad.lfb'],'ab');
else
    fid = fopen(full_name,'wb');
    fidfgrad = fopen([full_name(1:end-4),'_fgrad.lfb'],'wb');
    fidtgrad = fopen([full_name(1:end-4),'_tgrad.lfb'],'wb');
end

temp0 = [block_len;num_chans;afull];
temp1 = [foff;filt_lens];
lofb = 2*numel(temp0)+2*numel(temp1)+4*numel(fc)+4*numel(g_mat)+4;

% Write filterbank length
fwrite(fid,lofb,'uint32');
fwrite(fidfgrad,lofb,'uint32');
fwrite(fidtgrad,lofb,'uint32');

% Write filterbank parameters
fwrite(fid,temp0,'uint16');
fwrite(fidfgrad,temp0,'uint16');
fwrite(fidtgrad,temp0,'uint16');

fwrite(fid,fc,'float32');
fwrite(fidfgrad,fc,'float32');
fwrite(fidtgrad,fc,'float32');

fwrite(fid,temp1,'uint16');
fwrite(fidfgrad,temp1,'uint16');
fwrite(fidtgrad,temp1,'uint16');

% Write filter data
fwrite(fid,g_mat,'float32');
fwrite(fidfgrad,hg_mat,'float32');
fwrite(fidtgrad,dg_mat,'float32');

% Close file stream
fclose(fid);
fclose(fidfgrad);
fclose(fidtgrad);
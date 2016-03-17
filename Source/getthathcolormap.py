import matplotlib as mpl
import numpy 
from matplotlib import cm

cmaps = ('viridis','inferno','plasma','magma')

for cmapname in cmaps:
    cmapfnc = getattr(mpl.cm,cmapname)
    cmap = cmapfnc(range(0,256))

    with open(cmapname+'.m','w') as f:
        f.write('function ret = '+cmapname+'(varargin)\n')
        f.write('ret=[')
        f.writelines([ numpy.array_str(x[0:-1]) + '\n' for x in cmap])
        f.write('];')

"""Spike S2: does pushing f2/f1 toward 2.0 (and gamma up) raise second-register incidence? (Overblow control mechanism, D-006)"""
import json, numpy as np
from colinot_spike import run, f0, S
cnt={}
for f2 in [None,2.0]:
    k='orig_%.3f'%(S[1].imag/S[0].imag) if f2 is None else 'f2_2.000'
    c={'eq':0,'r1':0,'r2':0,'other':0}
    for g in [0.6,0.7,0.8]:
        for tau in [1e-4,3e-3,3e-2]:
            sig,Fs=run(g,tau,T=0.4,f2scale=f2); f,a=f0(sig,Fs)
            r='eq' if f==0 else ('r1' if abs(f-189)<15 else ('r2' if abs(f-380)<30 else 'other'))
            c[r]+=1
    cnt[k]=c; print(k,c,flush=True)
json.dump(cnt,open('overblow_map_result.json','w'),indent=1)

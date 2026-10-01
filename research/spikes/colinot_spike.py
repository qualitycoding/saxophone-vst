"""Spike S1: reproduce Colinot et al. 2021 (Acta Acust. 5:33) saxophone model, D# fingering.
Checks: (a) first register near 185 Hz at moderate gamma; (b) second register (~2x) reachable at higher gamma.
Scheme: exponential (exact) modal update with flow held over the step; reed by central differences; Fs = 176400."""
import numpy as np, sys, json
C = np.array([176.1,470.5,649.4,328.7,541.5,224.9,382.2,409.9])
S = np.array([-17.59+1195j,-35.50+2483j,-65.30+3727j,-269.34+4405j,-70.32+5153j,-166.0+6177j,-94.49+6749j,-116.5+7987j])
zeta, qr, wr, Kc, eta = 0.6, 1.0, 4224.0, 100.0, 1e-3
def run(gf, tau, T=0.6, Fs=176400, f2scale=None):
    S_ = S.copy()
    if f2scale is not None: S_[1] = S_[1].real + 1j*S[0].imag*f2scale
    dt=1/Fs; N=int(T*Fs); E=np.exp(S_*dt); G=C*(E-1)/S_
    pn=np.zeros(8,complex); x=0.0; xp=0.0; out=np.empty(N)
    for k in range(N):
        t=k*dt; g=gf/2*(1+np.tanh((t-5*tau)/tau))
        p=2*np.sum(pn.real)
        o=x+1; mn=(o-np.sqrt(o*o+eta))/2; Fc=Kc*mn*mn
        xn=(dt*dt*wr*wr*(p-g+Fc-x)+2*x-xp+0.5*qr*wr*dt*xp)/(1+0.5*qr*wr*dt)
        xp,x=x,xn
        o=x+1; mx=(o+np.sqrt(o*o+eta))/2; d=g-p
        u=zeta*mx*np.sign(d)*np.sqrt(np.sqrt(d*d+eta))
        pn=E*pn+G*u; out[k]=p
    return out,Fs
def f0(sig,Fs):
    s=sig[len(sig)//2:]; s=s-s.mean()
    if np.max(np.abs(s))<1e-3: return 0.0,0.0
    ac=np.correlate(s,s,'full')[len(s)-1:]; ac/=ac[0]
    lo=int(Fs/1200); hi=int(Fs/80); i=lo+np.argmax(ac[lo:hi]); return Fs/i, float(np.sqrt(np.mean(s*s)))
res={}
for gf,tau in [(0.5,0.010),(0.6,0.010),(0.75,0.0001),(0.9,0.0001),(0.9,0.003)]:
    sig,Fs=run(gf,tau); f,a=f0(sig,Fs); res[f"g{gf}_tau{tau}"]={"f0_hz":round(f,1),"rms":round(a,4)}
    print(gf,tau,res[f"g{gf}_tau{tau}"],flush=True)
json.dump(res,open('colinot_spike_result.json','w'),indent=1)

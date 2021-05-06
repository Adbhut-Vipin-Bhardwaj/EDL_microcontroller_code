clear
clc

dat = csvRead("C:\Users\Immanuel\Documents\SCILAB\EDL.csv");
frange = dat(2:41,1)
Mdb = dat(2:41,12)
phdeg = dat(2:41,16)
f1 = 100;
f2 = 1000;
eta1 = 0.1;
eta2 = 1;
//comp = 10*((1+%i.*frange./f2)./(1+%i.*frange./f1))^2;
comp = 2.5*((1+2.*eta2.*%i.*frange./f2 - (frange./f2)^2)./(1+2.*eta1.*%i.*frange./f1 - (frange./f1)^2));
Mcompdb = 20.*log10(abs(comp));
phcompdeg = atand(imag(comp),real(comp));
for i=1:length(phcompdeg)
if (phcompdeg(i,1)>0)
    phcompdeg(i,1) = phcompdeg(i,1) - 360;
end
end
nMdb = Mdb + Mcompdb;
nphdeg = phdeg + phcompdeg;
//figure
//plot("ln",frange,Mcompdb);
//figure
//plot("ln",frange,phcompdeg);
 
figure;
xgrid; f = gcf(); f.background = 8;
title("Bode Magnitude plot of Transfer functions T(s) and C(s)T(s)")
plot("ln",frange,Mdb,"r",frange,nMdb,"b");
xlabel("Frequency (Hz)")
ylabel("Gain (dB)")
legend(["T(s)","C(s)T(s)"])
figure
xgrid; f = gcf(); f.background = 8;
title("Bode Phase plot of Transfer functions T(s) and C(s)T(s)")
plot("ln",frange,phdeg,"r",frange,nphdeg,"b");
xlabel("Frequency (Hz)")
ylabel("Phase (deg)")
legend(["T(s)","C(s)T(s)"])

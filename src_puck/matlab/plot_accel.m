clear;
s = serial('/dev/ttyUSB5');
fopen(s);

N = 1500;
meanSamples=10;
values=[];
n=1;
tic;
while n < N+1
    t = fscanf(s,'%e')
    if length(t) == 1
        values(n) = t;
        n = n+1
    end
end
mdt=toc/N;

fclose(s);
%%
figure;plot((0:N-1)*mdt,values,'r');
hold on;

valuesM = reshape(values, meanSamples, length(values)/meanSamples);
meanM=mean(valuesM);
plot((0:(N-1)/10)*10*mdt, meanM,'b');
legend('X[n]','Mean X[m]')
xlabel('Samples') % x-axis label
ylabel('Accelerometer X axis') % y-axis label
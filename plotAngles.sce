
File = fullfile(pwd(), "data.csv");

data = csvRead(File);
temp = data;
dataSize = size (data)

// Some invertions to show graph as literature
for (i=1:1:dataSize(1))
    if (data(i,1) > 120) then     
        temp(i,1) = data(i,1) - 360
    end
end
temp(:,1) = -temp(:,1)

// 3d graph
x = sind(temp(:,1));
y = cosd(temp(:,1));
z = sind(90 - temp(:,2));

a = gca()
//a.rotation_angles=[65,75];
a.data_bounds=[-1,-1,0;1,1,1]; //boundaries given by data_bounds
a.thickness = 2;
param3d1(x,y,z);

//Azimuth and zenith graph
scf(1)
plot(temp (:,1), 90 - temp(:,2))

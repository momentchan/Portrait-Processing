 I2=I;
thresh = multithresh(I2,7);
valuesMax = [thresh max(I2(:))];
[quant8_I_max, index] = imquantize(I2,thresh,valuesMax);
valuesMin = [min(I2(:)) thresh];
quant8_I_min = valuesMin(index);
imwrite(quant8_I_max,'R2_quantize.jpg');
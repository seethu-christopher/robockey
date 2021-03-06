#include "localization.h"

// buffer for m_wii data
//unsigned int blobs[12] = {355,361,0,358,304,0,423,385,0,403,294,0};
unsigned int blobs[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int *x1 = &blobs[0]; unsigned int *y1_ = &blobs[1]; unsigned int *s1 = &blobs[2];
unsigned int *x2 = &blobs[3]; unsigned int *y2 = &blobs[4]; unsigned int *s2 = &blobs[5];
unsigned int *x3 = &blobs[6]; unsigned int *y3 = &blobs[7]; unsigned int *s3 = &blobs[8];
unsigned int *x4 = &blobs[9]; unsigned int *y4 = &blobs[10]; unsigned int *s4 = &blobs[11];

int localize(signed char location[]){
	int c = m_wii_read(blobs); // place data in buffer
	// create array of x and y points
	// flip the x-points to get right hand coordinate system
	int X[4] = {-1*(*x1), -1*(*x2) , -1*(*x3) ,-1*(*x4)}; 
	int Y[4] = {*y1_, *y2, *y3, *y4};
	if(DEBUG == 1)
	{	
		m_usb_tx_string("\nreturn value: ");
		m_usb_tx_int(c);
		m_usb_tx_string("\nblobs: ");
		m_usb_tx_int(blobs[0]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(X[1]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(X[2]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(X[3]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(Y[0]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(Y[1]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(Y[2]);
		m_usb_tx_string(" | ");
		m_usb_tx_int(Y[3]);
		m_usb_tx_string("\n");
	}
	// count the # of detected blobs
	int i = 0; int count = 0;
	static double theta;
	while (i < 4){
		if (X[i] != -1023 && Y[i] != 1023){
			count++;
		}
		i++;
	}
	if (count < 4) // if # of blobs is less than 4 return (don't change position)
	{	
		if(DEBUG){
			m_usb_tx_string("count 4");	
			//printf("inside count tin");		
		}
		return (int)theta;
	}
	// find all possible distances between points (in pixels)
	int p0[6] = {0,0,0,1,1,2};
	int p1[6] = {1,2,3,2,3,3};
	int d[6] = {};
	int x0_temp = 0; int x1_temp = 0;
	int y0_temp = 0; int y1_temp = 0;
	 
	for (i=0; i<6; i++){
		x0_temp = X[p0[i]]; y0_temp = Y[p0[i]];
		x1_temp = X[p1[i]]; y1_temp = Y[p1[i]];
		d[i] = sqrt((pow((x0_temp - x1_temp),2)) + (pow((y0_temp - y1_temp),2)));
		/*		
		printf("\nx0_temp: %d",x0_temp);
		printf("\ty0_temp: %d",y0_temp);
		printf("\tx1_temp: %d",x1_temp);
		printf("\ty1_temp: %d",y1_temp);	
		printf("\tdist: %d",d[i]);
		*/
	}
	
	// label points based on max and min distances
	int max =  0; int min = 1024;
	int max_points[2] = {0,0}; int min_points[2] = {0,0};
	for (i=0; i<6; i++){		
		if (d[i] < min){
			min = d[i];
			min_points[0] = p0[i];
			min_points[1] = p1[i];
		}
	}
	for(i=0;i<6;i++)
	{
		if (d[i] > max){
			max = d[i];
			max_points[0] = p0[i];
			max_points[1] = p1[i];
		}
	}
	/*	
	printf("\nx_min_point[0]: %d",X[min_points[0]]);
	printf("\nx_min_point[1]: %d",X[min_points[1]]);
	printf("\ny_min_point[0]: %d",Y[min_points[0]]);
	printf("\ny_min_point[1]: %d",Y[min_points[1]]);
	printf("\nx_max_point[0]: %d",X[max_points[0]]);
	printf("\nx_max_point[1]: %d",X[max_points[1]]);
	printf("\ny_max_point[0]: %d",Y[max_points[0]]);
	printf("\ny_max_point[1]: %d",Y[max_points[1]]);
	
	printf("\nmin_point[0]: %d",min_points[0]);
	printf("\nmin_point[1]: %d",min_points[1]);
	printf("\nmax_point[0]: %d",max_points[0]);
	printf("\nmax_point[1]: %d",max_points[1]);
	*/
	int j;
	int N_i=0,S_i=0;
	for (i=0; i<2; i++){
		for (j=0; j<2; j++){
			if (max_points[i] == min_points[j]){
				N_i = max_points[i];
				S_i = max_points[1-i];
				/*printf("\ni: %d",i);
				printf("\tS_i: %d",S_i);
				printf("\tN_i: %d",N_i);
				printf("\tX[N_i]: %d",X[N_i]);
				printf("\tY[N_i]: %d",Y[N_i]);
				printf("\tX[N_i]: %d",X[S_i]);
				printf("\tY[S_i]: %d",Y[S_i]);*/
			}
		}
	}
	
	// Find the center of the constellation and orientation
	int cx, cy, dir_x, dir_y;
	cx = abs((X[N_i] + X[S_i])/2);
	cy = abs((Y[N_i] + Y[S_i])/2);
	dir_x = X[N_i] - X[S_i];
	dir_y = Y[N_i] - Y[S_i];
	/*printf("\ncx: %d",cx);
	printf("\ncy: %d",cy);
	printf("\nx[ni]: %d",X[N_i]);
	printf("\ny[ni]: %d",Y[N_i]);
	printf("\nx[si]: %d",X[S_i]);
	printf("\ny[si]: %d",Y[S_i]);
	printf("\nni: %d",N_i);
	printf("\nsi: %d",S_i);
	printf("\ndir_x: %d",dir_x);
	printf("\ndir_y: %d",dir_y);*/
	theta = (double)atan2(dir_y,dir_x) - M_PI_2;
    
	if(theta < 0)
	{
		theta += 2*M_PI;
	}	

	// Calculate the scale for current image
	double scale = 29.0 / max; // cm / pix
	
	// Find location in global frame
	int dx = -511 + cx;
	int dy = 383 - cy;
	location[0] = dx * scale;
	location[1] = dy * scale;
	int loc_tmp = location[0];

	location[0] = cos(theta) * location[0] + sin(theta) * location[1];
	location[1] = -sin(theta) * loc_tmp + cos(theta) * location[1];
	
	theta *= (180 / M_PI); // convert to degrees
	return (int)theta;
}

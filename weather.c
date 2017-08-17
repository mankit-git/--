#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include "cJSON.h"
#include <stdbool.h>
#include <errno.h>

void show_weather_info(char *json)
{
	char *pos = json;
	while(1)
	{
		pos++;
		if(*pos == '{')
			break;
	}

	cJSON *root = cJSON_Parse(pos);
	cJSON *body = cJSON_GetObjectItem(root, "showapi_res_body");

	if(cJSON_GetObjectItem(body, "ret_code")->valueint == -1)
	{
		printf("%s\n", cJSON_GetObjectItem(body, "remark")->valuestring);
		return;
	}

	cJSON *now      = cJSON_GetObjectItem(body, "now");
	cJSON *cityInfo = cJSON_GetObjectItem(body, "cityInfo");
	cJSON *today    = cJSON_GetObjectItem(body, "f1");
	cJSON *tomorrow = cJSON_GetObjectItem(body, "f2");
	cJSON *day_3rd  = cJSON_GetObjectItem(body, "f3");

	char *country  = cJSON_GetObjectItem(cityInfo, "c9")->valuestring;
	char *province = cJSON_GetObjectItem(cityInfo, "c7")->valuestring;
	char *city     = cJSON_GetObjectItem(cityInfo, "c5")->valuestring;

	bool zhixiashi = !strcmp(city, province);
	printf("城市：%s.%s%s%s\n\n", country, province, zhixiashi ? "" : ".", zhixiashi ? "" : city);

	printf("现在天气：%s\n", cJSON_GetObjectItem(now, "weather")->valuestring);
	printf("现在气温：%s°C\n\n", cJSON_GetObjectItem(now, "temperature")->valuestring);

	printf("明天天气：%s\n", cJSON_GetObjectItem(tomorrow, "day_weather")->valuestring);
	printf("日间气温：%s°C\n", cJSON_GetObjectItem(tomorrow, "day_air_temperature")->valuestring);
	printf("夜间气温：%s°C\n\n", cJSON_GetObjectItem(tomorrow, "night_air_temperature")->valuestring);

	printf("后天天气：%s\n", cJSON_GetObjectItem(day_3rd, "day_weather")->valuestring);
	printf("日间气温：%s°C\n", cJSON_GetObjectItem(day_3rd, "day_air_temperature")->valuestring);
	printf("夜间气温：%s°C\n\n", cJSON_GetObjectItem(day_3rd, "night_air_temperature")->valuestring);
}

int main(int argc, char **argv)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct hostent *host = gethostbyname("ali-weather.showapi.com");
	if(host == NULL)
	{
		perror("gethostbyname() failed");
		exit(0);
	}
	char ip[100];
	if(inet_ntop(AF_INET, *(host->h_addr_list), ip, sizeof(ip)) == NULL)
	{
		perror("inet_ntop() failed");
		exit(0);
	}
	printf("ip: %s\n", ip);

	struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
	struct sockaddr_in svaddr;
	socklen_t len = sizeof(svaddr);
	bzero(&svaddr, len);
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(80);
	//svaddr.sin_addr.s_addr = inet_addr("42.120.88.82");
	svaddr.sin_addr = **addr_list;

	if(connect(fd, (struct sockaddr *)&svaddr, len) == -1)
	{
		perror("connect() failed");
		exit(0);
	}

	char buf[500];
	bzero(buf, 500);
	char *phone_code = argv[1];
	snprintf(buf, 500, "GET /phone-post-code-weeather?"
                            "phone_code=%s "
                            "HTTP/1.1\r\n"
                            "Host:ali-weather.showapi.com\r\n"
                            "Authorization:APPCODE d2f5c59144e5465ab13ed8986a42096d\r\n\r\n",
                            phone_code);
	write(fd, buf, 500);

	char msg[4096];
	bzero(msg, 4096);
	int n, m;
	
	n = read(fd, msg, 4096);
	printf("%d\n", n);
	m = read(fd, msg + n, 4096);
	read(fd, msg + n + m, 4096);
	show_weather_info(msg);

	/*while(1)
	{

	}*/
	
	return 0;
}

package com.chanjet.transnotice;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

@SpringBootApplication
@EnableAutoConfiguration
@EnableScheduling
public class TransnoticeApplication {

	public static void main(String[] args) {
		SpringApplication.run(TransnoticeApplication.class, args);
	}
}

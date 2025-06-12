package mabecker.qnx.codegeneration

import mabecker.qnx.codegeneration.model.QnxNode

class SysConfGen {
	
	static def generateHeader(String settingName, int runtime_ms, boolean recordExecTrace, boolean recordApsTrace, QnxNode node) {//ArrayList<Task> tasks, mabecker.qnx.codegeneration.AmaltheaWrapper wrapper, HwStructure ecu) {
		'''
			/*
			 * sysConfig.h
			 */
			
			#ifndef SYSCONFIG_H_
			#define SYSCONFIG_H_
			#include "thread.h"
			
			/**
			 * Bitmasks used to assign core affinities
			 */
			«FOR c : node.cores»
				#define CORE«c.id»	«Generator.generateCoreBitmap(c)»
			«ENDFOR»
			
			/**
			 * Definitions for partition IDs (Linux - simplified, no cgroups for minimal approach)
			 */
			«FOR p : node.partitions»
				#define «p.name.replace(' ', '_').toUpperCase»	«node.partitions.indexOf(p)»
			«ENDFOR»
			
			/**
			 * IP address of this node
			 */
			#define LOCAL_IP_ADDRESS 	"«node.ipAddress»"
			
			/**
			 * Name of the configuration. This is used as filename for the trace file.
			 */
			#define SETTING_NAME "«settingName»"
			
			/**
			 * APS tracing not supported in Linux version
			 */
			//#define TRACE_APS_BUDGETS
			/**
			 * Enable to collect the execution trace using tracelogger
			 */
			«IF recordExecTrace == true»
				#define TRACE_EXECUTION
			«ELSE»
				//#define TRACE_EXECUTION
			«ENDIF»
			
			/**
			 * Duration of the experiment in ms
			 */
			#define EXPERIMENT_RUNTIME_MS «runtime_ms»
			
			/**
			 * APS not supported in Linux version - simplified approach
			 */
			// No APS partitions in Linux version
			
			#define REGISTER_THREADS  \
			«FOR t : node.allTasks»
				thread_addThreadDescription("«t.name»", «t.period_ms», «t.deadline_ms», 0, «t.priority», «Generator.generateCoreAffinityString(t)», «t.partition.name.replace(' ', '_').toUpperCase»);\
			«ENDFOR»
				/* Register runnables for all tasks */\
			«FOR t : node.allTasks»
				«FOR r : t.runnables»
					thread_registerRunnable("«t.name»", &«r.name»);\
				«ENDFOR»
			«ENDFOR»
				
			#endif /* SYSCONFIG_H_ */
			
		'''
	}
}

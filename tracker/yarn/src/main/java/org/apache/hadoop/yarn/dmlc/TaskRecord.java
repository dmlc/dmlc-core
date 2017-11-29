package org.apache.hadoop.yarn.dmlc;

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */


import org.apache.hadoop.yarn.api.records.Container;
import org.apache.hadoop.yarn.client.api.AMRMClient.ContainerRequest;

/**
 * data structure to hold the task information
 */
public class TaskRecord {
    // task id of the task
    public int taskId = 0;
    // role of current node 
    public String taskRole = "worker";
    // number of failed attempts to run the task
    public int attemptCounter = 0;
    // container request, can be null if task is already running
    public ContainerRequest containerRequest = null;
    // running container, can be null if the task is not launched
    public Container container = null;
    // whether we have requested abortion of this task
    public boolean abortRequested = false;

    public TaskRecord(int taskId, String role) {
        this.taskId = taskId;
        this.taskRole = role;
    }
}

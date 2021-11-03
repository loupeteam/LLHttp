

pipeline {
    agent {
    // Specify a label that defines the type of agent that can be used for this build
    label 'windows-automation-studio'
    }
    options {
        timestamps() // This appends timestamps to the console output in Jenkins
        quietPeriod(0) // This specifies the amount of time Jenkins waits between receiving a webhook trigger and starting the job
        ansiColor('xterm')
    }
    stages {
        // stage('Install CMake') {
        //     steps {
                
        //     }
        // }
        stage('Build Project') {
            steps {
                cmakeBuild buildDir: 'build', buildType: 'Debug', installation: 'InSearchPath', sourceDir: './'
            }
        }
        // stage('Run Tests') {
            
        // }
    }
    post {
        always {
            script {
                    slackSend(channel: "#sandbox-github", message: "LLHttp ran")
                }
            }
    }
}

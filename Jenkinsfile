pipeline {
  agent any

  stages {
    stage('Checkout') {
      steps {
        checkout scm
      }
    }

    stage('Configure CMake') {
      steps {
        sh 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Release'
      }
    }

    stage('Build') {
      steps {
        sh 'cmake --build build -j'
      }
    }

    stage('GoogleTest / CTest') {
      steps {
        sh 'ctest --test-dir build --output-on-failure'
      }
    }

    stage('Benchmarks') {
      steps {
        sh './build/dispatch_lookup_structure_benchmark'
        sh 'python3 performance_systems/cache_locality_experiment.py'
        sh 'python3 performance_systems/kv_cache_batching_sim.py'
        sh 'python3 performance_systems/threading_microbenchmark.py'
      }
    }

    stage('Regression Gate') {
      steps {
        sh 'bash ci/quality_gate.sh'
      }
    }

    stage('Archive Reports') {
      steps {
        archiveArtifacts artifacts: 'ci/*.md,reports/**/*.md,reports/**/*.html,performance_systems/*.md,execution_engine_lab/*.md', allowEmptyArchive: true
      }
    }
  }

  post {
    always {
      sh 'git status --short || true'
    }
  }
}

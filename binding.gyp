{
  "targets": [
    {
      "target_name": "docker",
      "sources": ["src/docker_win.cpp"],
      "include_dirs": ["<!(node -e \"require('nan')\")"]
    }
  ]
}

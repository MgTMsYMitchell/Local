import * as THREE from 'three';

function main() {
  const canvas = document.createElement('canvas');
  document.body.style.margin = '0';
  document.body.appendChild(canvas);

  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
  renderer.setSize(window.innerWidth, window.innerHeight);

  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x050509);

  const camera = new THREE.PerspectiveCamera(
    45,
    window.innerWidth / window.innerHeight,
    0.1,
    100
  );
  camera.position.set(0, 0, 5);

  const light = new THREE.DirectionalLight(0xffffff, 1.0);
  light.position.set(1, 1, 1);
  scene.add(light);

  const geom = new THREE.CylinderGeometry(0.1, 0.2, 2, 16);
  const mat = new THREE.MeshStandardMaterial({ color: 0x334455 });
  const trunk = new THREE.Mesh(geom, mat);
  scene.add(trunk);

  function onResize() {
    renderer.setSize(window.innerWidth, window.innerHeight);
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
  }
  window.addEventListener('resize', onResize);

  function animate() {
    requestAnimationFrame(animate);
    trunk.rotation.y += 0.002;
    renderer.render(scene, camera);
  }
  animate();
}

main();

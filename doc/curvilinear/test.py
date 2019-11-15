import numpy as np
import numpy.linalg as la
import unittest

import coordinates

class TestCartesian(unittest.TestCase):
  def setUp(self):
    self.sys = coordinates.Cartesian(3)
    self.x = np.array([1.2,2.4,-0.8])

  def test_scalar_grad(self):
    s = 1.2
    ds = [1.4, 0.8, 0.2]
    self.assertTrue(np.allclose(ds, self.sys.grad_scalar(self.x, s, ds)))

  def test_vector_grad(self):
    v = [1.4,0.8,0.2]
    dv = np.array([
      [0.8,0.7,-0.2],
      [-0.25,0.3,0.1],
      [1.2,6.3,-1.2]])
    self.assertTrue(np.allclose(dv, self.sys.grad_vector(self.x, v, dv)))

class TestCylindrical(unittest.TestCase):
  def setUp(self):
    self.sys = coordinates.Cylindrical()
    self.x = np.array([1.2,2.4,-0.8])

  def test_scalar_grad(self):
    s = 1.2
    ds = [1.4, 0.8, 0.2]
    self.assertTrue(np.allclose([ds[0],ds[1]/self.x[0], ds[2]],
      self.sys.grad_scalar(self.x, s, ds)))
  
  def test_vector_grad(self):
    v = [1.,1,1]
    dv = np.zeros((3,3))
    r = self.x[0]

    ans = np.array([
      [dv[0,0], (dv[0,1]-v[1]) / r, dv[0,2]],
      [dv[1,0], dv[1,1]/r + v[0]/r, dv[1,2]],
      [dv[2,0], dv[2,1]/r, dv[2,2]]])

    print(ans)
    print(self.sys.grad_vector(self.x, v, dv))

    self.assertTrue(np.allclose(ans, self.sys.grad_vector(self.x, v, dv)))
